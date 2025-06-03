
ROOT_DIR = ../
BUILD_DIR = ${ROOT_DIR}/build

# binary name
APP = ${BUILD_DIR}/nginz_plugin.a

# all source are stored in SRCS
SRCS := plugin.cc
OBJS := $(SRCS:.cc=.o)


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
	$(CXX) $(CXXFLAGS) $(CFLAGS) -c $^ -o $@

build:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -f $(OBJS) ${APP}
