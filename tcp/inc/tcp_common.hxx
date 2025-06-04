
#ifndef NGINZ_TCP_COMMON_H
#define NGINZ_TCP_COMMON_H


#include <deque>

#include <rte_mbuf.h>

#include "reactor.hxx"

#include "tcp_header.hxx"
#include "tcp_control_block.hxx"
#include "tcp_state_machine.hxx"
#include "fixedtable.hxx"

namespace nginz
{

namespace tcp
{

#define MAX_TCP_CLIENT 64

//! \brief given client ip and port build an unique identifier
inline uint64_t makeClientId(uint32_t ipv4Addr, uint16_t port)
{
    uint64_t ret = ipv4Addr;
    ret <<= 16;
    ret |= port;
    return ret;
}

struct tcpImpl final {

    nginz::FixedDict< uint64_t,TcpControlBlock, MAX_TCP_CLIENT > tcbTable;
    //std::unordered_map<uint64_t, TcpControlBlock> tcbTable;

    TcpControlBlock listen {80}; // hard coded

    void handleTcpRx(rte_mbuf*m);
};

enum {
    FATAL = 1,
    ERROR,
    WARN,
    INFO,
    DEBUG,
};


}
}

#define TCP_LOG(LEVEL, FMT, ...) printf("TCP:" FMT "\n" __VA_OPT__(, __VA_ARGS__))


extern nginz::tcp::tcpImpl gl_tcpCtxt[nginz::MAX_THREADS]; 

#endif // NGINZ_TCP_COMMON_H
