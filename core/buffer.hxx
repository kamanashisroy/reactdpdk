
#ifndef NGINZ_BUFFER_H
#define NGINZ_BUFFER_H

#include <rte_mbuf.h>
#include <variant>

namespace nginz
{
// use automatic garbage collection.

template<typename CONTENT>
inline uint16_t readUseCount(const CONTENT*x) { return x->refcnt;}

template<typename CONTENT>
inline void updateUseCount(CONTENT*x, int16_t val) { x->refcnt+=val;}

template <
    typename CONTENT
    , void DEALLOC_CB(CONTENT*)=free
    , uint16_t READ_CB(const CONTENT*)=readUseCount<CONTENT>
    , void UPDATE_CB(CONTENT*, int16_t )=updateUseCount<CONTENT> >
struct aroop_autobuf final {

    using SELF = aroop_autobuf<CONTENT,DEALLOC_CB,READ_CB,UPDATE_CB>;

    aroop_autobuf() : data(nullptr) {
    }

    
    aroop_autobuf(CONTENT*data) {
        reset(data);
    }

    aroop_autobuf(const SELF &) = delete;
    aroop_autobuf& operator=(const SELF&) = delete;

    aroop_autobuf(SELF&&given) {
        if(&given == this) {return;}

        auto& self = *this;
        clear();
        self.data = given.data;
        given.data = nullptr;
    }

    aroop_autobuf& operator=(aroop_autobuf&&given) {
        if(&given == this) {return *this;}

        auto& self = *this;
        clear();
        self.data = given.data;
        given.data = nullptr;
        return *this;
    }

    ~aroop_autobuf() { clear(); }

    void ownWithoutIncrement(CONTENT*givenData) {
        auto&self = *this;
        self.reset(nullptr);
        self.data = givenData;
    }

    void clear() {
        auto&self = *this;

        if(self.data) {
            auto cnt = READ_CB(self.data);
            if(1 == cnt)
            {
                DEALLOC_CB(self.data);
            }
            UPDATE_CB(self.data, -1);
            self.data = nullptr; // FIXME there could be a leak in inter-thread messaing when two thread owns it.
        }
    }

    void reset(CONTENT*givenData) {
        auto&self = *this;

        if(givenData == self.data )
        {
            return;
        }
        clear();
        // TODO assert not zero
        UPDATE_CB(self.data, 1);
        self.data = givenData;
    }

    CONTENT*get()
    {
        auto&self = *this;
        return self.data;
    }

    CONTENT*release()
    {
        auto&self = *this;
        auto*output = self.data;
        self.data = nullptr;
        return output;
    }
    
    CONTENT*data = nullptr;
};

using rte_autobuf = aroop_autobuf<rte_mbuf, rte_pktmbuf_free, rte_mbuf_refcnt_read, rte_pktmbuf_refcnt_update>;

struct aroop_txt_t final {
    using aroop_txt_content = std::variant<rte_autobuf, char*>;
	uint16_t size;
	uint16_t len;
	uint64_t hash;
	aroop_txt_content content;

    uint64_t hashCode() {
        // auto&self = *this;
        // FILLME
        return hash;
    }
};

// TODO use likey/unlikely optimization here
struct RteMbufIterator final {

    using value_type = uint8_t;

    RteMbufIterator(rte_mbuf*given):m(given) {}

    RteMbufIterator& operator++() {
        auto&self = *this;
        if(self.m)
        {
            if(self.offset < rte_pktmbuf_data_len(self.m))
            {
                self.offset++;
            }
            else
            {
                self.offset = 0;
                self.m = self.m->next;
            }
        }
        return self;
    }

    RteMbufIterator& skip(const uint32_t givenInc) {
        auto&self = *this;
        auto inc = givenInc;
        while(self.m and inc > 0)
        {
            if( (self.offset+inc) < rte_pktmbuf_data_len(self.m))
            {
                self.offset+=inc;
                break;
            }
            else
            {
                inc -= rte_pktmbuf_data_len(self.m)-self.offset;
                self.offset = 0;
                self.m = self.m->next;
            }
        }
        return self;
    }



    uint8_t operator*() {
        auto&self = *this;
        return *( ((uint8_t*)self.m->buf_addr)+self.m->data_off + self.offset );
    }

    bool eof() const {
        auto&self = *this;
        return 0 == self.size();
    }

    std::size_t size() const {
        auto&self = *this;
        if(not self.m) {
            return 0;
        }
        auto pktlen = rte_pktmbuf_pkt_len(self.m);
        assert(self.pkt_offset <= pktlen );
        return pktlen-self.pkt_offset;
    }

    rte_mbuf*   m = nullptr;
    std::size_t offset = 0;
    std::size_t pkt_offset = 0; // used to calculate size
};

struct RteMbufStream final {

    RteMbufStream(rte_mbuf* given):m(given)
    {
    }

    RteMbufIterator begin()
    {
        auto&self = *this;
        return {self.m};
    }

    RteMbufIterator end()
    {
        return {nullptr};
    }

    rte_mbuf* m = nullptr;
};

struct RteMbufReader final {

    RteMbufReader(rte_mbuf* given):itr(given)
    {
    }

    
    RteMbufReader& operator>>(char& c) {
        auto&self = *this;
        if(self.itr.eof()) {
            return self;
        }
        
        c = (char)*self.itr;
        ++self.itr;
        return self;
    }

    RteMbufReader& operator>>(uint8_t& outv) {
        auto&self = *this;
        if(self.itr.eof()) {
            return self;
        }
        
        outv = (uint8_t)*self.itr;
        ++self.itr;
        return self;
    }

    RteMbufReader& operator>>(uint16_t& outv) {
        auto&self = *this;
        if(self.itr.size() < sizeof(outv)) {
            return self;
        }
        

        uint8_t outhi,outlo;
        self >> outhi >> outlo;

        outv   = outhi;
        outv <<= 8;
        outv   |= outlo;
        return self;
    }

    RteMbufReader& operator>>(uint32_t& outv) {
        auto&self = *this;
        if(self.itr.size() < sizeof(outv)) {
            return self;
        }
        

        uint16_t outhi,outlo;
        self >> outhi >> outlo;

        outv   = outhi;
        outv <<= 16;
        outv   |= outlo;
        return self;
    }

    RteMbufReader& operator>>(uint64_t& outv) {
        auto&self = *this;
        if(self.itr.size() < sizeof(outv)) {
            return self;
        }
        
        uint32_t outhi,outlo;
        self >> outhi >> outlo;

        outv   = outhi;
        outv <<= 32;
        outv   |= outlo;
        return self;
    }

    void skip(uint32_t numBytes)
    {
        auto&self = *this;
        self.itr.skip(numBytes);
    }

    RteMbufIterator itr;
};

rte_mempool *get_tx_pool();


template<typename T, const size_t CAPACITY=128>
struct Arr {
    using Iterator = T*;

    Iterator begin()
    {
        return &data[0];
    }

    Iterator end()
    {
        return &data[cnt];
    }

    void push_back(T given) {
        auto&self = *this;
        
        assert(cnt < CAPACITY);
        self.data[self.cnt] = given;
        self.cnt++;
    }


    void emplace(T&&given) {
        auto&self = *this;
        
        assert(cnt < CAPACITY);
        self.data[self.cnt] = std::move(given);
        self.cnt++;
    }

    T& back(size_t ridx=0) {
        auto& self = *this;
        assert(self.cnt > ridx and self.cnt < CAPACITY);
        return self.data[self.cnt-1-ridx];
    }

    T& operator[](size_t idx) {
        auto& self = *this;
        assert(self.cnt > idx);
        return self.data[idx];
    }

    bool empty() const {
        return 0 == cnt;
    }

    std::size_t size() const {
        return cnt;
    }
    T data[CAPACITY];
    std::size_t cnt = 0;
};

//using aroop_txt_t = aroop_txt;
//typedef struct aroop_txt aroop_txt_t;
}

#endif
