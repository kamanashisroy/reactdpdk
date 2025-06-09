
ROOT_DIR = ../
BUILD_DIR = ${ROOT_DIR}/build
CORE_DIR = ${ROOT_DIR}/core

# binary name
APP = ${BUILD_DIR}/tcp_plugin.a

# all source are stored in SRCS
SRCS = tcp_plugin.cc \
	tcp_rx.cc \
	tcp_control_block.cc \
	tcp_tx.cc \
	fsm/tcp_state_machine_listen.cc \
	fsm/tcp_state_machine_syn_sent.cc \
	fsm/tcp_state_machine_syn_rcvd.cc \
	fsm/tcp_state_machine_established.cc \
	fsm/tcp_state_machine_close_wait.cc \
	fsm/tcp_state_machine_last_ack.cc \
	fsm/tcp_state_machine_fin_wait_1.cc \
	fsm/tcp_state_machine_fin_wait_2.cc \
	fsm/tcp_state_machine_closing.cc \
	fsm/tcp_state_machine_time_wait.cc

OBJS := $(SRCS:.cc=.o)

INCLUDES := -Iinc -I${CORE_DIR}

# Build using pkg-config variables if possible
ifneq ($(shell pkg-config --exists libdpdk && echo 0),0)
$(error "no installation of DPDK found")
endif

all: ${APP}

PKGCONF ?= pkg-config

# TODO O3 
CFLAGS += $(shell $(PKGCONF) --cflags libdpdk)
CXXFLAGS+=-ggdb3 -std=c++17 -Wno-subobject-linkage -fpermissive
CFLAGS += -DALLOW_EXPERIMENTAL_API

${APP}:${OBJS}
	ar -rcs $@ $^

%.cc:Makefile

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CFLAGS) ${INCLUDES} -c $^ -o $@

build:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -f $(OBJS) ${APP}
