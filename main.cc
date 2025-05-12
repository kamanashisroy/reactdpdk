#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>



#include <rte_common.h>
#include "reactdpdk_config.h"
#include "syslog.h"
#include <rte_mbuf.h>
#include <variant>
//#include "rte_mbuf.h"


#include "log.h"
#include <signal.h>
#include "buffer.hxx"
#include "reactor.hxx"
#include "plugin.h"


static int
lcore_launch(__rte_unused void *arg)
{
	unsigned lcore_id;
	lcore_id = rte_lcore_id();

    nginz::pm_run(lcore_id);
	return 0;
}

int
main(int argc, char **argv)
{
	int ret;
	unsigned lcore_id;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_base", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	nginz::pm_init();

	/* call lcore_init() on every worker lcore */
    rte_eal_mp_remote_launch(lcore_launch, nullptr, SKIP_MAIN);
	/*RTE_LCORE_FOREACH_WORKER(lcore_id) {
		rte_eal_remote_launch(lcore_init, NULL, lcore_id);
	}*/

	/* call it on main lcore too */
	//lcore_init(NULL);

	rte_eal_mp_wait_lcore();

	nginz::pm_deinit();
	closelog();
	return 0;
}

