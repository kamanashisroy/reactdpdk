

#include <rte_ether.h>

#include "tcp_common.hxx"
//#include "tcp_rx.hxx"



using namespace nginz;
using namespace nginz::tcp;

nginz::tcp::tcpImpl gl_tcpCtxt[nginz::MAX_THREADS]; 

constexpr uint32_t TCP_HEADER_NUM_DEFAULT_WORD32 = 5;

void tcpImpl::handleTcpRx(rte_mbuf*m)
{
    auto&self = *this;
    rte_autobuf rbuf;
    rbuf.ownWithoutIncrement(m);

    auto*iphdr = (struct rte_ipv4_hdr *)
        rte_pktmbuf_adj(m, (uint16_t)sizeof(struct rte_ether_hdr));
    RTE_ASSERT(iphdr != NULL);
    auto dst_addr = rte_be_to_cpu_32(iphdr->dst_addr);
    auto src_addr = rte_be_to_cpu_32(iphdr->src_addr);
    auto pkt_size = rte_be_to_cpu_16(iphdr->total_length);
    auto ip_header_len = rte_ipv4_hdr_len(iphdr);

    // now let us get the tcp header
    RteMbufReader reader(m);
    reader.skip(sizeof(struct rte_ether_hdr)+ip_header_len);

    TcpSegmentHeader header;
    reader >> header.sport >> header.dport >> header.seqno >> header.ackno;
    uint16_t left = 0;
    reader >> left;
    header.evtmask = left&0xF;
    // 4 bit data offset
    header.data_offset = 0xF & (left>>12);
    if(header.data_offset < TCP_HEADER_NUM_DEFAULT_WORD32)
    {
        TCP_LOG(WARN,"Invalid packet data offset");
        return ;
    }
    header.num_option_bytes = 4*(header.data_offset-TCP_HEADER_NUM_DEFAULT_WORD32);

    reader >> header.supp_window_size >> header.checksum >> header.uptr;

    reader.skip(header.num_option_bytes);

    header.total_header_bytes = sizeof(struct rte_ether_hdr)+ip_header_len+header.num_option_bytes;

    // now the reader is at data-offset
    
    // find the tcp control block in the table
    auto tgt = self.tcbTable.find(header.dport);
    if(not tgt) {
        // handle connection accept
        TCP_LOG(WARN,"destination port [%d] not found", header.dport);
    }
    else {
        // feed into state machine
        // TODO validate tcp fsm state 
        if(tgt->second.state >= TCP_STATE_MAX)
        {
            // TODO clear state-machine
            self.tcbTable.erase(header.dport);            
            // make the pool available
            tgt->second.changeState(TCP_STATE_EXCEPTION_HANG);
            return;
        }
        gl_tcpFsm[tgt->second.state](tgt->second, header, rbuf.release());
    }
}

