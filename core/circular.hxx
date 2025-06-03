

#ifndef NGINZ_CIRCULAR_H
#define NGINZ_CIRCULAR_H

#include <atomic>

namespace nginz
{


template <typename T, const size_t CAPACITY>
struct bus_mp_sc {

    T front()
    {
        auto&self = *this;
        auto rp = self.readPos.load(std::memory_order_acquire);
        return self.data[rp];
    }

    T back()
    {
        auto&self = *this;

        auto wp = self.writePos.load(std::memory_order_acquire);
        return self.data[(wp+CAPACITY-1)%CAPACITY];
    }
    
    bool empty() // used by consumer
    {
        auto&self = *this;
        auto wp = self.writePos.load(std::memory_order_acquire);
        auto rp = self.readPos.load(std::memory_order_relaxed);
        return ((rp+1)%CAPACITY) == wp;
    }

    int push(T given) {
        auto&self = *this;

        auto wp = self.writePos.load(std::memory_order_relaxed);
        auto rp = self.readPos.load(std::memory_order_acquire);
        if( unlikely(wp == rp) )
        {
            return -1; // drop packet
        }

        self.data[wp] = given;
        self.writePos.store((wp+1)%CAPACITY, std::memory_order_release);
        return 0;
    }

    void pop_front() {
        auto&self = *this;

        auto rp = self.readPos.load(std::memory_order_release);
        self.readPos.store((rp+1)%CAPACITY, std::memory_order_release);
    }

    std::atomic<size_t> readPos = 0;
    T data[CAPACITY];
    std::atomic<size_t> writePos = 1;
};

}

#endif // NGINZ_CIRCULAR_H
