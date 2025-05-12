#ifndef NGINZ_PLUGIN_H
#define NGINZ_PLUGIN_H

namespace nginz
{

enum {
    SERVICE_BASE_ENUMERATE=0,
    SERVICE_BASE_MAX,
};

enum {
    MSG_BASE_SVC_INIT = 0,
    MSG_BASE_SVC_DEINIT,
    MSG_BASE_COMMAND_SHOW,
    MSG_BASE_COMMAND_HELP,
    MSG_BASE_COMMAND_MQ_SHOW,
    MSG_BASE_MAX,
};


int pm_init();
int pm_deinit();
int pm_run(int coreIdx);

}

#endif // NGINZ_PLUGIN_H
