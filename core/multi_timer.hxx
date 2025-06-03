
#ifndef MULTI_TMR_HXX
#define MULTI_TMR_HXX

#include "reactor.hxx"
#include "rte_timer.h"

namespace nginz
{

struct mtimer_unit
{
    //uint32_t tmrEvent = 0;
    //uint32_t tmrMs = 0;
    struct rte_timer tmrObj;

    mtimer_unit()
    {
        auto& self = *this;
        rte_timer_init(&self.tmrObj);
    }

    ~mtimer_unit()
    {
        auto& self = *this;
        self.stop();
    }

    bool start(uint32_t givenTmrEvent, uint32_t givenMs, rte_timer_cb_t givenTimerCb, uintptr_t srcAddr )
    {
        auto& self = *this;
        uint64_t numTicks = givenMs*100;

        if(0 == numTicks)
        {
            return false;
        }


        if(self.isRunning())
        {
            self.stop();
        }

        uintptr_t evt = (0xFF&givenTmrEvent);
        srcAddr |= evt<<56; // put the event in highest byte

        //self.tmrMs = 0;
        //self.tmrEvent = 0;
        if( 0 == rte_timer_reset(
                        &self.tmrObj,
                        numTicks,           //!< number of ticks
                        SINGLE,           //!< timer type
                        g_this_threadId,    //!< core id
                        givenTimerCb,       //!< call back
                        (void*)srcAddr))
        {
            //tmrMs = givenMs;
            //tmrId = givenTmrId;
            return true;
        }
        return false;
    }

    bool isRunning()
    {
        auto& self = *this;
        return RTE_TIMER_STOP != self.tmrObj.status.state;
    }

    void stop()
    {
        auto& self = *this;

        if(self.isRunning())
        {
            rte_timer_stop(&self.tmrObj);
        }
    }
};

template <const int MAX_TMR, const rte_timer_cb_t TMR_CALLBACK>
struct multi_timer
{
    std::array<mtimer_unit, MAX_TMR> tmrs;
    uintptr_t                                       srcAddr = 0;
    
    
    void setSrcAddr(uintptr_t givenSrcAddr)
    {
        auto& self = *this;
        self.srcAddr = givenSrcAddr;
        // TODO reser all timers here
    }

    template <const unsigned TMR_ID>
    bool start(uint32_t givenMs) {
        static_assert(TMR_ID < MAX_TMR);
        static_assert(TMR_ID <= 0xFF);
        auto& self = *this;
        return self.tmrs[TMR_ID].start(TMR_ID, givenMs, TMR_CALLBACK, self.srcAddr);
    }

    template <const unsigned TMR_ID>
    void stop() {
        static_assert(TMR_ID < MAX_TMR);
        static_assert(TMR_ID <= 0xFF);
        auto& self = *this;
        if(self.tmrs[TMR_ID].has_value())
        {
            self.tmrs[TMR_ID].reset();
        }
    }

    template <const unsigned TMR_ID>
    bool isRunning() {
        static_assert(TMR_ID < MAX_TMR);
        static_assert(TMR_ID <= 0xFF);
        auto& self = *this;
        if(self.tmrs[TMR_ID].has_value())
        {
            return self.tmrs[TMR_ID].isRunning();
        }
        return false;
    }

};

}

#endif // MULTI_TMR_HXX
