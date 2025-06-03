

#include "tcp_common.hxx"
#include "tcp_tx.hxx"

using namespace nginz;
using namespace nginz::tcp;


void nginz::tcp::sendSynAck(TcpControlBlock&tcb, TcpSegmentHeader&header)
{
    rte_mbuf*m = nullptr;

    // TODO
    //reactorPost(g_this_threadId, SERVICE_BASE_MAIN, SERVICE_PROTO_TCP, MSG_BASE_SEND_PACKET, m);
}

void nginz::tcp::sendFinAck(TcpControlBlock&tcb, TcpSegmentHeader&header)
{
    rte_mbuf*m = nullptr;

    // TODO
    //reactorPost(g_this_threadId, SERVICE_BASE_MAIN, SERVICE_PROTO_TCP, MSG_BASE_SEND_PACKET, m);
}

void nginz::tcp::sendFin(TcpControlBlock&tcb, uint16_t seqno)
{
    // TODO
    //reactorPost(g_this_threadId, SERVICE_BASE_MAIN, SERVICE_PROTO_TCP, MSG_BASE_SEND_PACKET, m);
}

