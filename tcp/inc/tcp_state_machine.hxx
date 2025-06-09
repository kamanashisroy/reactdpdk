
#ifndef NGINZ_TCP_STATE_MACHINE_HXX
#define NGINZ_TCP_STATE_MACHINE_HXX

#include "buffer.hxx"

namespace nginz
{
namespace tcp
{

using tcp_fsmCallback = std::function<void (TcpControlBlock&, TcpSegmentHeader&head, rte_autobuf m)>;
using tcp_fsmTmr = std::function< void (TcpControlBlock&, uint8_t tmrEvt)>;

template <const int>
tcp_fsmCallback make_tcpFsmCallback() 
{
    return [] (TcpControlBlock&tcb, TcpSegmentHeader&header, rte_autobuf m) {
    };
}


template <const int>
tcp_fsmTmr make_tcpFsmTmr()
{

    return [] (TcpControlBlock&tcb, uint8_t tmrId) -> void {
    };
}


}
}


extern nginz::tcp::tcp_fsmCallback gl_tcpFsm[];

extern nginz::tcp::tcp_fsmTmr gl_tcpFsmTmr[];

#endif // NGINZ_TCP_STATE_MACHINE_HXX
