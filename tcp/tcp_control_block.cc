

#include "tcp_common.hxx"


using namespace nginz;
using namespace nginz::tcp;

TcpControlBlock::TcpControlBlock(uint64_t givenClientId) : clientId(givenClientId)
{
    tmr.setSrcAddr(givenClientId);
}

void TcpControlBlock::changeState(tcp_state new_state)
{
    auto&self = *this;
    auto existing_state = self.state;
    if(self.state == new_state)
    {
        return; // ignore
    }
    TCP_LOG(INFO, "Change state [%d] > [%d]", existing_state, new_state);
    self.state = new_state;

    if(TCP_STATE_EXCEPTION_HANG == new_state)
    {
        self.tmr.start<TCP_FINAL_TMR_2>(TCP_CONTROL_BLOCK_DEFAULT_MS);
    }
}

void nginz::tcp::onTcpCbTimerExpire(struct rte_timer *tmrObj, void *cbPtr)
{
    uintptr_t srcAddr = (uintptr_t)cbPtr;

    auto tmrEvt = srcAddr>>56; // TODO avoid magic number
    uint64_t timerMask = 0xFF;
    timerMask <<= 56;
    uint64_t clientId = (~timerMask) & srcAddr;
    auto tcb = gl_tcpCtxt[g_this_threadId].tcbTable.find(clientId);
    if(not tcb)
    {
        TCP_LOG(ERROR, "Invalid timer expire on port [%d] ", clientId);
        return;
    }

    tcb->second.processTimer(tmrEvt&0xFF);
}

void TcpControlBlock::processTimer(uint8_t evtId)
{
    auto& self = *this;
    gl_tcpFsmTmr[self.state](self, evtId);
}

