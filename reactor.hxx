
#ifndef NGINZ_REACTOR_H
#define NGINZ_REACTOR_H

namespace nginz
{

constexpr int MAX_SERVICES = 16;
constexpr int MAX_THREADS = 8;

// service == reactor == event_loop
using service_id_t = uint8_t;
using core_id_t = uint8_t; // core == thread

constexpr bool isServiceEnabled(service_id_t svcId) {
    return NGINZ_SERVICE_FLAG & (1<<svcId);
}

#pragma pack(push, 1) // Ensure no padding is added
struct MsgHeader final
{
    uint8_t srcThd : 4;
    uint8_t srcSvc : 4;
    uint8_t dstThd : 4;
    uint8_t dstSvc : 4;
};
#pragma pack(pop) // Restore previous packing

//===========================================================================
//! \brief Sender side invokation API that can go to any thread.
int reactorPost(
    core_id_t targetThread
    , service_id_t tgtSrv
    , service_id_t srcSrv
    , int msgId
    , rte_mbuf *pkt
);
//===========================================================================

//===========================================================================
//! \brief Reactor callback on reciever side
//! #### Flow diagram
//! while (has-incoming-message) -> reactor(...)
struct Reactor {
    virtual ~Reactor() {}
    virtual int processMsg(service_id_t srcThd, service_id_t srcSvc, int msgId, rte_mbuf *pkts) = 0;
};
//===========================================================================


}

#endif // NGINZ_REACTOR_H
