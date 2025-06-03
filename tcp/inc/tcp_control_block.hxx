
#ifndef NGINZ_TCP_CONTROL_BLOCK
#define NGINZ_TCP_CONTROL_BLOCK

#include "buffer.hxx"
#include "circular.hxx"
#include "multi_timer.hxx"

#include "rte_ip.h"

namespace nginz
{

namespace tcp
{

enum tcp_state {
    TCP_STATE_CLOSED = 0,
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RCVD,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_CLOSING,
    TCP_STATE_LAST_ACK,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_TIME_WAIT,
    TCP_STATE_EXCEPTION_HANG,
    TCP_STATE_MAX,
};

/*constexpr char* TCP_STATE_STR[] = {
    "CLOSED",
    "LISTEN",
    "SYN_SENT",
    "SYN_RCVD",
    "ESTABLISHED",
    "CLOSE_WAIT",
    "FIN_WAIT_1",
    "CLOSING",
    "FIN_WAIT_2",
    "TIME_WAIT",
    "EXCEPTION_HANG"
};*/

//using tcp_addr = struct sockaddr_in;
using tcp_addr = rte_ipv4_hdr;

#define TCP_MAX_UNA_SEGMENTS 4098

struct tcp_segment
{
    uint32_t seqno;
    rte_autobuf pkt;
};

#define TCP_CONTROL_BLOCK_DEFAULT_MS 100

enum {
    TCP_CONNECTION_TMR = 1,
    TCP_INACTIVITY_TMR,
    TCP_FINAL_TMR_1,
    TCP_FINAL_TMR_2,
    TCP_MAX_TMR,
};

void onTcpCbTimerExpire(struct rte_timer *, void *);

struct TcpControlBlock final
{

    tcp_state       state = TCP_STATE_CLOSED;
    tcp_addr        laddr; //!< local address
    tcp_addr        raddr; //!< remote address
    uint16_t        sport; //!< source port
    uint16_t        dport; //!< destination port
    uint32_t        expseq; //!< expected sequence

    bus_mp_sc<tcp_segment, TCP_MAX_UNA_SEGMENTS> tx_seg;
    bus_mp_sc<tcp_segment, TCP_MAX_UNA_SEGMENTS> rx_seg;

    nginz::multi_timer<TCP_MAX_TMR, onTcpCbTimerExpire> tmr;

    TcpControlBlock();
    TcpControlBlock(uint16_t givenSrcPort, uint16_t givenDstPort);

    void changeState(tcp_state new_state);
    void processTimer(uint8_t evtIdx);
};

}

}


#endif // NGINZ_TCP_CONTROL_BLOCK
