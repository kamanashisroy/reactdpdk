
#ifndef NGINZ_TCP_TX_HXX
#define NGINZ_TCP_TX_HXX

namespace nginz
{

namespace tcp
{
    void sendSynAck(nginz::tcp::TcpControlBlock&tcb, TcpSegmentHeader&head);
    void sendFinAck(nginz::tcp::TcpControlBlock&tcb, TcpSegmentHeader&head);
    void sendFin(nginz::tcp::TcpControlBlock&tcb, uint16_t seqno);
}

}

#endif // NGINZ_TCP_TX_HXX
