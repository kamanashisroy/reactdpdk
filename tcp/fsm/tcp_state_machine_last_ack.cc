
#include "tcp_common.hxx"
#include "tcp_tx.hxx"

using namespace nginz;
using namespace nginz::tcp;

template <>
tcp_fsmCallback nginz::tcp::make_tcpFsmCallback<TCP_STATE_LAST_ACK>()
{
    return [] (TcpControlBlock&tcb, TcpSegmentHeader&header, rte_autobuf m) {
        assert(tcb.state == TCP_STATE_LAST_ACK);

        // TODO check if header.num_option_bytes is greater than equals m size
        if( (header.evtmask & tcp_mask::ACK) and (header.seqno == tcb.expseq) )
        {
            // return to pool
            gl_tcpCtxt[g_this_threadId].tcbTable.erase(header.sport);
            gl_tcpCtxt[g_this_threadId].availablePorts.push(header.sport);
            return;
        }
        else
        {
            TCP_LOG(WARN,"Invalid packet in listen state");
        }
    };
}

template <>
tcp_fsmTmr nginz::tcp::make_tcpFsmTmr<TCP_STATE_LAST_ACK>()
{

    return [] (TcpControlBlock&tcb, uint8_t tmrId) -> void {
        assert(tcb.state == TCP_STATE_LISTEN);
       
        switch(tmrId)
        {
            case TCP_FINAL_TMR_2:
                TCP_LOG(WARN, "timeout %d", tcb.sport);
                auto sport = tcb.sport;
                gl_tcpCtxt[g_this_threadId].tcbTable.erase(sport);
                gl_tcpCtxt[g_this_threadId].availablePorts.push(sport);
                break;
        } 
    };
}


