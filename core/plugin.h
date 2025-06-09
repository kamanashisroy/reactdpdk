#ifndef NGINZ_PLUGIN_H
#define NGINZ_PLUGIN_H

namespace nginz
{

enum {
    SERVICE_BASE_MAIN=0,
    SERVICE_BASE_ENUMERATE,
    SERVICE_BASE_MAX,
    SERVICE_PROTO_TCP = SERVICE_BASE_MAX,
};

enum {
    MSG_BASE_SVC_INIT = 0,
    MSG_BASE_SVC_DEINIT,
    MSG_BASE_SEND_PACKET,
    MSG_BASE_COMMAND_SHOW,
    MSG_BASE_COMMAND_HELP,
    MSG_BASE_COMMAND_MQ_SHOW,
    MSG_BASE_MAX,

    MSG_TCP_RX = MSG_BASE_MAX,
};


namespace pm
{

int init();
int deinit();
int run(int coreIdx);
void setServiceReactor(service_id_t svcId, Reactor reactorObj);

}

}

#endif // NGINZ_PLUGIN_H
