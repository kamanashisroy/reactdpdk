
#include "tcp_common.hxx"
#include "tcp_tx.hxx"

using namespace nginz;
using namespace nginz::tcp;

template <>
tcp_fsmCallback nginz::tcp::make_tcpFsmCallback<TCP_STATE_ESTABLISHED>()
{
    return [] (TcpControlBlock&tcb, TcpSegmentHeader&header, rte_autobuf m) {
        assert(tcb.state == TCP_STATE_ESTABLISHED);
        
        // TODO check if header.num_option_bytes is greater than equals m size
        if( (header.evtmask & tcp_mask::FIN) )
        {
            // send syn ack
            sendFinAck(tcb, header);
            
            // TODO start a connection timeout
            if(tcb.tmr.start<TCP_FINAL_TMR_1>(TCP_CONTROL_BLOCK_DEFAULT_MS))
            {
                tcb.expseq = header.seqno; // should we set this here ?
                tcb.changeState(TCP_STATE_CLOSE_WAIT);
            }
            return;
        }
        else
        {
            TCP_LOG(TCP_WARN,  "[ESTABLISHED] Invalid packet");
        }
    };
}

template <>
tcp_fsmTmr nginz::tcp::make_tcpFsmTmr<TCP_STATE_ESTABLISHED>()
{

    return [] (TcpControlBlock&tcb, uint8_t tmrId) -> void {
        assert(tcb.state == TCP_STATE_ESTABLISHED);
       
        switch(tmrId)
        {
            case TCP_INACTIVITY_TMR:
                // TODO send fin
                TCP_LOG(WARN, "[ESTABLISHED] TODO close inactive connection %lld", tcb.clientId);
                break;
        } 
    };
}



