
#include <rte_common.h>
#include "reactdpdk_config.h"
#include "syslog.h"
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include "rte_malloc.h"

#include <variant>
#include <atomic>
//#include "rte_mbuf.h"

//#include "packet.h"
//#include "ip.h"
//#include "dev.h"

#include "buffer.hxx"
#include "reactor.hxx"
#include "plugin.h"

//#include <rte_port_sched.h>

thread_local int g_this_threadId = 0;
volatile int g_quit = 0;

using namespace nginz;


// TODO put these to config.h.in

#ifndef THREAD_PIPELINES_MAX
#define THREAD_PIPELINES_MAX                               256
#endif

#ifndef THREAD_MSGQ_SIZE
#define THREAD_MSGQ_SIZE                                   64
#endif

#ifndef THREAD_TIMER_PERIOD_MS
#define THREAD_TIMER_PERIOD_MS                             100
#endif

#ifndef MAX_PKT_BURST
#define MAX_PKT_BURST 128
#endif

#ifndef RX_PORT_PER_CORE
#define RX_PORT_PER_CORE 1
#endif

#if 0 
#define MEMPOOL_CACHE_SIZE 512
#define RTE_TEST_TX_DESC_DEFAULT 1024
#define RTE_TEST_RX_DESC_DEFAULT 1024
#else
#define MEMPOOL_CACHE_SIZE 32
#define RTE_TEST_TX_DESC_DEFAULT 64
#define RTE_TEST_RX_DESC_DEFAULT 64
#endif

#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */

template <typename T, const size_t CAPACITY>
struct bus_mp_sc {

    T front()
    {
        auto&self = *this;
        auto rp = self.readPos.load(std::memory_order_acquire);
        return self.data[rp];
    }

    T back()
    {
        auto&self = *this;

        auto wp = self.writePos.load(std::memory_order_acquire);
        return self.data[(wp+CAPACITY-1)%CAPACITY];
    }
    
    bool empty() // used by consumer
    {
        auto&self = *this;
        auto wp = self.writePos.load(std::memory_order_acquire);
        auto rp = self.readPos.load(std::memory_order_relaxed);
        return ((rp+1)%CAPACITY) == wp;
    }

    int push(T given) {
        auto&self = *this;

        auto wp = self.writePos.load(std::memory_order_relaxed);
        auto rp = self.readPos.load(std::memory_order_acquire);
        if( unlikely(wp == rp) )
        {
            return -1; // drop packet
        }

        self.data[wp] = given;
        self.writePos.store((wp+1)%CAPACITY, std::memory_order_release);
        return 0;
    }

    void pop_front() {
        auto&self = *this;

        auto rp = self.readPos.load(std::memory_order_release);
        self.readPos.store((rp+1)%CAPACITY, std::memory_order_release);
    }

    std::atomic<size_t> readPos = 0;
    T data[CAPACITY];
    std::atomic<size_t> writePos = 1;
};

#ifndef MAX_RX_QUEUE_PER_LCORE
#define MAX_RX_QUEUE_PER_LCORE 16
#endif
#define MAX_TX_QUEUE_PER_PORT 16




struct NoneReactor : public Reactor {

    ~NoneReactor()
    {
    }
    int processMsg(service_id_t srcThd, service_id_t srcSvc, int msgId, rte_mbuf *pkts) override {
        syslog(LOG_ERR, "Error invalid reactor\n");
        return -1;
    }
} noneReactor;

//!
//! For more information please refer to https://doc.dpdk.org/guides/nics/overview.html
//!
struct PortInfo final {
    uint32_t portId = 0;
    uint64_t tx = 0;
    uint64_t rx = 0;
    uint64_t dropped = 0;

    //! Tx buffer
    //! defined in rte_ethdev.h
    //! More information here https://doc.dpdk.org/guides/nics/overview.html
    rte_eth_dev_tx_buffer *tx_buffer = nullptr;

    PortInfo() = default;
    PortInfo(uint32_t givenPort) : portId(givenPort) {};
};


// please refer to the DPDK example/pipeline/thread.c application
struct internal_thread final {
	//struct rte_ring *mbuf_mpsc = nullptr; // TODO put this in auto-pointer
    bus_mp_sc<rte_mbuf*,THREAD_MSGQ_SIZE> mbuf_mpsc;

    Arr<PortInfo, MAX_RX_QUEUE_PER_LCORE> portList; // similar to l2fwd_poll.c

	Arr<struct rte_mbuf*,MAX_PKT_BURST> pkts_burst;

    Reactor*srv[MAX_SERVICES] = {0};
	bool enabled = false;
    uint64_t prev_tsc = 0;
	uint64_t drain_tsc = 0;

    ~internal_thread()
    {
        // auto cleanup
        deinit();
    }


    int init(int instId)
    {
        auto&self = *this;
        if(self.enabled)
        {
            return 0;
        }

#if 0 // TODO
        self.mbuf_mpsc = rte_ring_create( "thread_mbuf_mpsc",
                    THREAD_MSGQ_SIZE,
                    instId,
                    RING_F_MP_RTS_ENQ | RING_F_SC_DEQ); // TODO review these flags

        if (not self.mbuf_mpsc ) {
            assert(!"mbuf ring creation failed");
            return -1;
        }
#endif
        for(service_id_t i = 0; i < MAX_SERVICES; i++)
        {
            self.srv[i] = &noneReactor;
        }
        self.enabled = true;
        syslog(LOG_NOTICE, "Successfully init the thread [%d]\n", instId);
        return 0;
    }

    int deinit()
    {
        auto&self = *this;
        /* take care of partial initialization if(not self.enabled)
        {
            return 0;
        }*/

		/*if (self.mbuf_mpsc) {
			rte_ring_free(self.mbuf_mpsc);
        }*/

        for(service_id_t i = 0; i < MAX_SERVICES; i++)
        {
            // FILLME invoke deinit
        }
        // FILLME
        return 0;
    }

    void addPort(uint32_t portId) {
        auto&self = *this;
        self.portList.push_back(portId);

        /* Initialize TX buffers */
		auto* tx_buffer = (rte_eth_dev_tx_buffer *)rte_zmalloc_socket("tx_buffer",
				RTE_ETH_TX_BUFFER_SIZE(MAX_PKT_BURST), 0,
				rte_eth_dev_socket_id(portId));
		if (tx_buffer == NULL) {
			rte_panic("Cannot allocate buffer for tx on port %u\n",
				  portId);
        }

		rte_eth_tx_buffer_init(tx_buffer,
				       MAX_PKT_BURST);

		auto ret = rte_eth_tx_buffer_set_err_callback(
				tx_buffer,
				rte_eth_tx_buffer_count_callback,
				&self.portList.back().dropped);
		if (ret < 0) {
			rte_panic("Cannot set error callback for tx buffer on port %u\n",
				  portId);
            return;
        }
        self.portList.back().tx_buffer = tx_buffer;

    }

    void processRx(rte_mbuf*m, uint32_t portId) {
        printf("Received packet \n");
    }

    void txrxBurst()
    { // follow l2fwd_poll.c 
        auto&self = *this;
		auto cur_tsc = rte_rdtsc();
		/*
		 * TX burst queue drain
		 */
		auto diff_tsc = cur_tsc - self.prev_tsc;
		if (unlikely(diff_tsc > self.drain_tsc)) {
            for( auto& portInfo : self.portList ) {
				auto portId = portInfo.portId;
                //assert(portId > 0);
                assert(portInfo.tx_buffer);
				auto sent = rte_eth_tx_buffer_flush(portId, 0, portInfo.tx_buffer);
				if (sent)
					portInfo.tx += sent;
			}

			self.prev_tsc = cur_tsc;
		}

		/*
		 * Read packet from RX queues
		 */
        for( auto& portInfo : self.portList ) {
			auto portId = portInfo.portId;
			auto nb_rx = rte_eth_rx_burst(portId, 0, self.pkts_burst.begin(),
						 MAX_PKT_BURST);

			portInfo.rx += nb_rx;
            self.pkts_burst.cnt = nb_rx;

            for(auto*m : self.pkts_burst) {
				rte_prefetch0(rte_pktmbuf_mtod(m, void *));
				self.processRx(m, portId);
			}
		}
    }


    int run(uint32_t lcoreId)
    {
        auto&self = *this;

	    self.drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S *
			BURST_TX_DRAIN_US;

        if (self.portList.empty()) {
            printf("lcore %u has nothing to do\n", lcoreId);
            return 0;
        }

        printf("entering main loop on lcore %u\n", lcoreId);

        for( auto& portInfo : self.portList ) {
            printf(" -- lcoreid=%u portId=%u\n", lcoreId, portInfo.portId);

        }



        while(!g_quit) {

            txrxBurst();
#if 0
            if(mbuf_mpsc.empty()) {
                return 0;
            }
            txrx_burst();
            auto* pkt = mbuf_mpsc.front();

            printf("Received a message TODO cleanUP");

            // TODO cleanup message
            mbuf_mpsc.pop_front();
#endif
        }
        return 0;
    }
};

static internal_thread g_threads[MAX_THREADS] {};



int reactorPost(const core_id_t targetThread, service_id_t dstSvc, service_id_t srcSvc, int msgId, rte_mbuf *pkts) {
    if(targetThread == g_this_threadId)
    {
        // use tight coupling
        auto ret = g_threads[g_this_threadId].srv[dstSvc]->processMsg(g_this_threadId, srcSvc, msgId, pkts);
        rte_pktmbuf_free(pkts);
        return ret;
    }

    assert(pkts);
    MsgHeader header;
    header.srcThd = g_this_threadId; // source thread
    header.srcSvc = srcSvc; // source service

    header.dstThd = targetThread; // target thread
    header.dstSvc = dstSvc; // target service

    auto*buff = rte_pktmbuf_prepend(pkts, sizeof(header));
    assert(buff);
    memcpy(buff, &header, sizeof(header));
    // use loose coupling
    //return rte_ring_mp_enqueue(g_threads[targetThread].mbuf_mpsc, pkts);
    return g_threads[targetThread].mbuf_mpsc.push(pkts);
}

#define RX_DEFAULT_RINGSIZE 1024
#define TX_DEFAULT_RINGSIZE 1024

rte_mempool*gl_mbuf_pool = nullptr;
static uint16_t gl_nb_rxd = RX_DEFAULT_RINGSIZE;
static uint16_t gl_nb_txd = TX_DEFAULT_RINGSIZE;
static rte_ether_addr ioat_ports_eth_addr[RTE_MAX_ETHPORTS];

#define PORT_CONFIG_MASK 1111111
/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
static inline void
port_init(uint16_t portid, struct rte_mempool *mbuf_pool, uint16_t nb_queues, uint16_t lcoreId)
{
	/* configuring port to use RSS for multiple RX queues */
	static const struct rte_eth_conf port_conf = {
		.rxmode = {
			.mq_mode = ETH_MQ_RX_RSS,
			.max_rx_pkt_len = RTE_ETHER_MAX_LEN
		},
		.rx_adv_conf = {
			.rss_conf = {
				.rss_key = NULL,
				.rss_hf = ETH_RSS_PROTO_MASK,
			}
		}
	};

	struct rte_eth_rxconf rxq_conf;
	struct rte_eth_txconf txq_conf;
	struct rte_eth_conf local_port_conf = port_conf;
	struct rte_eth_dev_info dev_info;
	int ret, i;

	/* Skip ports that are not enabled */
	if ((PORT_CONFIG_MASK & (1 << portid)) == 0) {
		printf("Skipping disabled port %u\n", portid);
		return;
	}

	/* Init port */
	printf("Initializing port %u... ", portid);
	fflush(stdout);
	ret = rte_eth_dev_info_get(portid, &dev_info);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Cannot get device info: %s, port=%u\n",
			rte_strerror(-ret), portid);

	local_port_conf.rx_adv_conf.rss_conf.rss_hf &=
		dev_info.flow_type_rss_offloads;
	if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
		local_port_conf.txmode.offloads |=
			DEV_TX_OFFLOAD_MBUF_FAST_FREE;
	ret = rte_eth_dev_configure(portid, nb_queues, 1, &local_port_conf);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Cannot configure device:"
			" err=%d, port=%u\n", ret, portid);

	ret = rte_eth_dev_adjust_nb_rx_tx_desc(portid, &gl_nb_rxd,
			&gl_nb_txd);
	if (ret < 0)
		rte_exit(EXIT_FAILURE,
			"Cannot adjust number of descriptors: err=%d, port=%u\n",
			ret, portid);

	rte_eth_macaddr_get(portid, &ioat_ports_eth_addr[portid]);

	/* Init RX queues */
	rxq_conf = dev_info.default_rxconf;
	rxq_conf.offloads = local_port_conf.rxmode.offloads;
	for (i = 0; i < nb_queues; i++) {
		ret = rte_eth_rx_queue_setup(portid, i, gl_nb_rxd,
			rte_eth_dev_socket_id(portid), &rxq_conf,
			mbuf_pool);
		if (ret < 0)
			rte_exit(EXIT_FAILURE,
				"rte_eth_rx_queue_setup:err=%d,port=%u, queue_id=%u\n",
				ret, portid, i);
	}

	/* Init one TX queue on each port */
	txq_conf = dev_info.default_txconf;
	txq_conf.offloads = local_port_conf.txmode.offloads;
	ret = rte_eth_tx_queue_setup(portid, 0, gl_nb_txd,
			rte_eth_dev_socket_id(portid),
			&txq_conf);
	if (ret < 0)
		rte_exit(EXIT_FAILURE,
			"rte_eth_tx_queue_setup:err=%d,port=%u\n",
			ret, portid);

	/* Initialize TX buffers */
    g_threads[lcoreId].addPort(portid);

	/* Start device */
	ret = rte_eth_dev_start(portid);
	if (ret < 0)
		rte_exit(EXIT_FAILURE,
			"rte_eth_dev_start:err=%d, port=%u\n",
			ret, portid);

	rte_eth_promiscuous_enable(portid);

	printf("Port %u, MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
			portid,
			ioat_ports_eth_addr[portid].addr_bytes[0],
			ioat_ports_eth_addr[portid].addr_bytes[1],
			ioat_ports_eth_addr[portid].addr_bytes[2],
			ioat_ports_eth_addr[portid].addr_bytes[3],
			ioat_ports_eth_addr[portid].addr_bytes[4],
			ioat_ports_eth_addr[portid].addr_bytes[5]);

	//cfg.ports[cfg.nb_ports].rxtx_port = portid;
	//cfg.ports[cfg.nb_ports++].nb_queues = nb_queues;
}


int nginz::pm_init() {

    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(g_threads[i].init(i))
        {
            syslog(LOG_ERR, "Failed to init the thread [%d]\n", i);
            assert(!"Thread init failed");
        }
    }

	auto nb_ports = rte_eth_dev_count_avail();
	if (nb_ports == 0)
		rte_panic("No Ethernet ports - bye\n");

#if 0 
	auto nb_mbufs = RTE_MAX(nb_ports * (RTE_TEST_RX_DESC_DEFAULT +
				       RTE_TEST_TX_DESC_DEFAULT +
				       MAX_PKT_BURST + rte_lcore_count() *
				       MEMPOOL_CACHE_SIZE), 8192U);
#endif

	auto nb_mbufs = nb_ports * (RTE_TEST_RX_DESC_DEFAULT +
				       RTE_TEST_TX_DESC_DEFAULT +
				       MAX_PKT_BURST + rte_lcore_count() *
				       MEMPOOL_CACHE_SIZE);


    printf("NB MBUFS %d\n", nb_mbufs);
	/* create the mbuf pool */
	gl_mbuf_pool = rte_pktmbuf_pool_create("mbuf_pool",
			nb_mbufs, MEMPOOL_CACHE_SIZE, 0,
			RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (gl_mbuf_pool == NULL)
		rte_panic("Cannot init mbuf pool\n");



	uint32_t rx_lcore_id = 0;
	uint16_t port_id = 0;

	/* Initialize the port/queue configuration of each logical core */
	RTE_ETH_FOREACH_DEV(port_id) {
		/* skip ports that are not enabled */
		if ((PORT_CONFIG_MASK & (1 << port_id)) == 0)
			continue;
		if (!rte_eth_dev_is_valid_port(port_id)) {
			printf("port %u is not valid\n", port_id);
			return -1;
		}

		/* get the lcore_id for this port */
		while (rte_lcore_is_enabled(rx_lcore_id) == 0 ||
		       rx_lcore_id == rte_get_main_lcore() ||
		       g_threads[rx_lcore_id].portList.size() == MAX_RX_QUEUE_PER_LCORE) {
			rx_lcore_id++;
            assert(rx_lcore_id < MAX_THREADS);
			if (rx_lcore_id >= RTE_MAX_LCORE)
				rte_panic("Not enough cores\n");

		}

        assert(rx_lcore_id < MAX_THREADS);
        port_init(port_id, gl_mbuf_pool, 1/* numb queue */ , rx_lcore_id);

		printf("Lcore %u: RX port %u\n", rx_lcore_id, port_id);
	}
	printf("pm_init():Initialized all threads\n");

	return 0;
}

int nginz::pm_deinit() {
    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(g_threads[i].deinit()) {
            syslog(LOG_ERR, "Failed to deinitialize the thread [%d]\n", i);
        }
    }

	return 0;
}

int nginz::pm_run(int coreIdx) {

    g_this_threadId = coreIdx;

	printf("Running event loop %u\n", g_this_threadId);
    g_threads[g_this_threadId].run(coreIdx);
    return 0;
}
