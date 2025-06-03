
#include "tcp_common.hxx"
#include "tcp_tx.hxx"

using namespace nginz;
using namespace nginz::tcp;

template <>
tcp_fsmCallback nginz::tcp::make_tcpFsmCallback<TCP_STATE_FIN_WAIT_1>()
{
    return [] (TcpControlBlock&tcb, TcpSegmentHeader&header, rte_autobuf m) {
        assert(tcb.state == TCP_STATE_FIN_WAIT_1);
    };
}

template <>
tcp_fsmTmr nginz::tcp::make_tcpFsmTmr<TCP_STATE_FIN_WAIT_1>()
{

    return [] (TcpControlBlock&tcb, uint8_t tmrId) -> void {
        assert(tcb.state == TCP_STATE_FIN_WAIT_1);
    };
}


