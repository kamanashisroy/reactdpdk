
#include "tcp_common.hxx"
#include "tcp_tx.hxx"

using namespace nginz;
using namespace nginz::tcp;

template <>
tcp_fsmCallback nginz::tcp::make_tcpFsmCallback<TCP_STATE_SYN_RCVD>()
{
    return [] (TcpControlBlock&tcb, TcpSegmentHeader&header, rte_autobuf m) {
        assert(tcb.state == TCP_STATE_SYN_RCVD);
        
        // TODO check if header.num_option_bytes is greater than equals m size
        if( (header.evtmask & tcp_mask::ACK) and (header.seqno == tcb.expseq) )
        {
            // send syn ack
            sendSynAck(tcb, header);
            
            // TODO start a connection timeout
            if(tcb.tmr.start<TCP_INACTIVITY_TMR>(TCP_CONTROL_BLOCK_DEFAULT_MS*100))
            {
                tcb.expseq = header.seqno; // should we set this here ?
                tcb.changeState(TCP_STATE_ESTABLISHED);
            }
            return;
        }
        else
        {
            TCP_LOG(TCP_WARN,  "[SYN_RCVD] Invalid packet");
        }
    };
}

template <>
tcp_fsmTmr nginz::tcp::make_tcpFsmTmr<TCP_STATE_SYN_RCVD>()
{

    return [] (TcpControlBlock&tcb, uint8_t tmrId) -> void {
        assert(tcb.state == TCP_STATE_SYN_RCVD);
       
        switch(tmrId)
        {
            case TCP_CONNECTION_TMR:
                TCP_LOG(WARN, "[SYN_RCVD]Connection timer expired on listening addr %lld ", tcb.clientId);
                // TODO send fin
                break;
        } 
    };
}



