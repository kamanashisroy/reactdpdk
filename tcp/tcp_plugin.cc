
#include "tcp_common.hxx"
#include "tcp_plugin.hxx"
#include "plugin.h"


using namespace nginz;
using namespace nginz::tcp;

nginz::tcp::tcp_fsmCallback gl_tcpFsm[] = {
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_CLOSED>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_LISTEN>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_SYN_SENT>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_SYN_RCVD>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_ESTABLISHED>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_CLOSE_WAIT>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_FIN_WAIT_1>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_CLOSING>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_LAST_ACK>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_FIN_WAIT_2>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_TIME_WAIT>(),
    nginz::tcp::make_tcpFsmCallback<nginz::tcp::TCP_STATE_EXCEPTION_HANG>()
};


nginz::tcp::tcp_fsmTmr gl_tcpFsmTmr[] = {
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_CLOSED>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_LISTEN>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_SYN_SENT>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_SYN_RCVD>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_ESTABLISHED>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_CLOSE_WAIT>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_FIN_WAIT_1>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_CLOSING>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_LAST_ACK>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_FIN_WAIT_2>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_TIME_WAIT>(),
    nginz::tcp::make_tcpFsmTmr<nginz::tcp::TCP_STATE_EXCEPTION_HANG>()
};

static nginz::Reactor TCP_REACTOR = [] (service_id_t srcThd, service_id_t srcSvc, int msgId, rte_mbuf *pkts) -> int {

    switch(msgId)
    {
        case MSG_TCP_RX:
            TCP_LOG(DEBUG, "Received TCP packet");
            gl_tcpCtxt[g_this_threadId].handleTcpRx(pkts);
            break;
    }
    // FIXME cleanup pkt if unprocessed
    return 0;
};

int nginz::pm::tcp_init()
{
    nginz::pm::setServiceReactor(SERVICE_PROTO_TCP, TCP_REACTOR);

    for(int i = 0; i < MAX_THREADS; i++)
    {
        for(uint16_t j = TCP_CLIENT_PORT_BEGIN; j < (TCP_CLIENT_PORT_BEGIN+MAX_TCP_CLIENT); j++) {
            gl_tcpCtxt[i].availablePorts.push(j);
        }
    }


    return 0;
}

int nginz::pm::tcp_deinit()
{
    return 0;
}

