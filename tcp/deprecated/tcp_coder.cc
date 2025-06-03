

#include "tcp_header.h"


#define ENCODE8(cur, tgt8) \
    assert(cur+1 <= end); \
    *((uint8_t*)cur) = tgt8; \
    cur += 1


#define ENCODE16(cur, tgt16) \
    assert(cur+2 <= end); \
    *((uint16_t*)cur) = tgt16; \
    cur += 2

#define ENCODE32(cur, tgt32) \
    assert(cur+4 <= end); \
    *((uint32_t*)cur) = tgt32; \
    cur += 4

int tcp_coder::encode_header(tcp_header&given, uint8_t*buf, uint32_t size)
{
    auto*end = &buf[size];

    auto*cur = buf;

    ENCODE16(cur, given.sport);
    ENCODE16(cur, given.dport);
    ENCODE32(cur, given.seqno);
    ENCODE32(cur, given.ackno);

    uint16_t left = 0;
    // 4 bit data offset
    left = (given.data_offset & 0xF) << 8;
    left |= given.typemask;
    ENCODE16(cur, left);

    // though it is taking 16 bits, it is recommended we keep 32bit value.
    ENCODE16(cur, given.supp_window_size); 

    ENCODE16(cur, given.checksum); 
    ENCODE16(cur, given.uptr); 

    for(int i = 0; i < (0xF&given.data_offset); i++){
        ENCODE8(cur, given.options[i]); 
    }
    // TODO emit error code if there is not enough buffer size
    return 0;
}


#define DECODE8(cur, tgt8) \
    assert(cur+1 <= end); \
    tgt8 = *((uint8_t*)cur); \
    cur += 1


#define DECODE16(cur, tgt16) \
    assert(cur+2 <= end); \
    tgt16 = *((uint16_t*)cur); \
    cur += 2

#define DECODE32(cur, tgt32) \
    assert(cur+4 <= end); \
    tgt32 = *((uint32_t*)cur); \
    cur += 4

int tcp_coder::decode_header(tcp_header&given, uint8_t*buf, uint32_t size)
{
    auto*end = &buf[size];

    auto*cur = buf;

    DECODE16(cur, given.sport);
    DECODE16(cur, given.dport);
    DECODE32(cur, given.seqno);
    DECODE32(cur, given.ackno);

    uint16_t left = 0;
    DECODE16(cur, left);
    given.typemask = left&0xF;
    // 4 bit data offset
    given.data_offset = 0xF & (left>>12);

    // though it is taking 16 bits, it is recommended we keep 32bit value.
    DECODE16(cur, given.supp_window_size); 

    DECODE16(cur, given.checksum); 
    DECODE16(cur, given.uptr); 

    for(int i = 0; i < (0xF&given.data_offset); i++){
        DECODE8(cur, given.options[i]); 
    }
    // TODO emit error code if there is not enough buffer size
    return 0;
}
