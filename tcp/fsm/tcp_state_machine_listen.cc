
#include "tcp_common.hxx"
#include "tcp_tx.hxx"

using namespace nginz;
using namespace nginz::tcp;

template <>
tcp_fsmCallback nginz::tcp::make_tcpFsmCallback<TCP_STATE_LISTEN>()
{
    return [] (TcpControlBlock&tcb, TcpSegmentHeader&header, rte_autobuf m) {
        assert(tcb.state == TCP_STATE_LISTEN);

        if(gl_tcpCtxt[g_this_threadId].availablePorts.empty())
        {
            // TODO send failure
            
            TCP_LOG(ERROR, "No empty ports");
            return;
        }
        
        // TODO check if header.num_option_bytes is greater than equals m size
        if(header.evtmask & tcp_mask::SYN)
        {


            auto clientPort = gl_tcpCtxt[g_this_threadId].availablePorts.front();
            gl_tcpCtxt[g_this_threadId].availablePorts.pop_front();
            //auto nblock = TcpControlBlock(header.dport, clientPort);
            //auto ret = gl_tcpCtxt[g_this_threadId].tcbTable.emplace( {clientPort, std::move(nblock) } );

            auto ret = gl_tcpCtxt[g_this_threadId].tcbTable.emplace( clientPort, header.dport, clientPort );
            if(not ret)
            {
                TCP_LOG(ERROR, "Unable to insert into tcp table");
                // TODO send failure
                gl_tcpCtxt[g_this_threadId].availablePorts.push(clientPort);
                return;
            }
            auto& connCb = ret->second;

            connCb.expseq = header.seqno+1; 

            // send syn ack
            sendSynAck(connCb, header);
            
            // TODO start a connection timeout
            if(connCb.tmr.start<TCP_CONNECTION_TMR>(TCP_CONTROL_BLOCK_DEFAULT_MS))
            {
                connCb.changeState(TCP_STATE_SYN_SENT);
            }
            else
            {
                TCP_LOG(ERROR, "Could not start the timer");
                // TODO send failure 
                gl_tcpCtxt[g_this_threadId].tcbTable.erase(clientPort);
                gl_tcpCtxt[g_this_threadId].availablePorts.push(clientPort);
            }
            return;
        }
        else
        {
            TCP_LOG(TCP_WARN, "[LISTEN]Invalid packet");
        }
    };
}

template <>
tcp_fsmTmr nginz::tcp::make_tcpFsmTmr<TCP_STATE_LISTEN>()
{

    return [] (TcpControlBlock&tcb, uint8_t tmrId) -> void {
        assert(tcb.state == TCP_STATE_LISTEN);
       
        switch(tmrId)
        {
            case TCP_CONNECTION_TMR:
                TCP_LOG(WARN, "Connection timer expired on port %d", tcb.sport);
                // TODO send fin
                break;
        } 
    };
}



