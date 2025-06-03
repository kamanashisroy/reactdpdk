#ifndef NGINZ_CONFIG_H
#define NGINZ_CONFIG_H

#define NGINZ_INLINE inline

/**
 * Parallel computing
 */
// XXX token ring parallel processing requires that there are at least 2 processes
enum parallel_config {
	NGINZ_NUMBER_OF_PROCESSORS = 5,
};

enum binary_coder_config {
	NGINZ_MAX_BINARY_MSG_LEN = 1024,
};

#include <poll.h> // to define POLLIN, POLLPRI, POLLHUP
/**
 * Event loop configuration
 */
enum event_loop_config {
	MAX_POLL_FD = 20000, // for some implementation this value must by devisible by some POLL_PARTITION value
	NGINZ_POLL_LISTEN_FLAGS = POLLIN | POLLPRI | POLLHUP,
	NGINZ_POLL_ALL_FLAGS = POLLIN | POLLPRI | POLLHUP,
	NGINZ_POLL_CLOSE_FLAGS = POLLHUP,
};
/**
 * Lazy call API
 */
enum {
	NGINZ_LAZY_STACK_SIZE = 24,
	NGINZ_LAZY_OBJECT_QUEUE_SIZE = 1024,
};

/**
 * Protocol implemenation
 */
enum protocol_config {
	NGINZ_MAX_PROTO = 4,
	NGINZ_CHAT_PORT = 9399,
	NGINZ_HTTP_PORT = 80,
};

/**************************************************************************/
/***************************** enable modules *****************************/
/**************************************************************************/
// http module allows to load http
#define HAS_HTTP_MODULE

// chat module allows telnet chat
#define HAS_CHAT_MODULE
// web chat module tunnels chat over htp
#define HAS_WEB_CHAT_MODULE
//#define HAS_MEMCACHED_MODULE


// fixup
#ifndef HAS_CHAT_MODULE
#undef HAS_WEB_CHAT_MODULE
#endif
#ifndef HAS_HTTP_MODULE
#undef HAS_WEB_CHAT_MODULE
#endif
/**************************************************************************/


#endif // NGINZ_CONFIG_H
