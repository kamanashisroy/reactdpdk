
#include "tcp_common.hxx"
#include "tcp_tx.hxx"

using namespace nginz;
using namespace nginz::tcp;

template <>
tcp_fsmCallback nginz::tcp::make_tcpFsmCallback<TCP_STATE_CLOSE_WAIT>()
{
    return [] (TcpControlBlock&tcb, TcpSegmentHeader&header, rte_autobuf m) {
        assert(tcb.state == TCP_STATE_CLOSE_WAIT);

        // TODO check if header.num_option_bytes is greater than equals m size
        if( (header.evtmask & tcp_mask::ACK) and (header.seqno == tcb.expseq) )
        {
            // send syn ack
            tcb.expseq++;
            sendFin(tcb, tcb.expseq);
            
            if(tcb.tmr.start<TCP_FINAL_TMR_2>(TCP_CONTROL_BLOCK_DEFAULT_MS))
            {
                tcb.changeState(TCP_STATE_LAST_ACK);
            }
            else
            {
                gl_tcpCtxt[g_this_threadId].tcbTable.erase(header.sport);
                gl_tcpCtxt[g_this_threadId].availablePorts.push(header.sport);
                TCP_LOG(ERROR," failed to start timer ");
            }
            return;
        }
        else
        {
            TCP_LOG(WARN,"Invalid packet in listen state");
        }
    };
}

template <>
tcp_fsmTmr nginz::tcp::make_tcpFsmTmr<TCP_STATE_CLOSE_WAIT>()
{

    return [] (TcpControlBlock&tcb, uint8_t tmrId) -> void {
        assert(tcb.state == TCP_STATE_LISTEN);
       
        switch(tmrId)
        {
            case TCP_FINAL_TMR_1:
                // send syn ack
                sendFin(tcb, tcb.expseq+1);
                tcb.expseq++;
                
                TCP_LOG(WARN, "Connection fin tmr 1 expired on port %d", tcb.sport);
                tcb.changeState(TCP_STATE_LAST_ACK);
                break;
        } 
    };
}


