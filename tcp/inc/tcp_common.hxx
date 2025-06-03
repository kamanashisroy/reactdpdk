
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

#define TCP_CLIENT_PORT_BEGIN 4000
#define MAX_TCP_CLIENT 64


struct tcpImpl final {

    nginz::FixedDict< uint16_t,TcpControlBlock, MAX_TCP_CLIENT > tcbTable;
    //std::unordered_map<uint16_t, TcpControlBlock> tcbTable;

    //! Free list
    nginz::bus_mp_sc<uint16_t, MAX_TCP_CLIENT>                               availablePorts;

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

#define TCP_LOG(LEVEL, FMT, ...) printf("TCP" FMT __VA_OPT__(, __VA_ARGS__))


extern nginz::tcp::tcpImpl gl_tcpCtxt[nginz::MAX_THREADS]; 

#endif // NGINZ_TCP_COMMON_H
