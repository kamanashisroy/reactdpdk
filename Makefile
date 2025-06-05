# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2010-2014 Intel Corporation

# binary name
APP = reactordpdk

SUBDIRS := $(wildcard core tcp)

# all source are stored in SRCS
SRCS := main.cc 

LIBS := build/nginz_plugin.a build/tcp_plugin.a

# Build using pkg-config variables if possible
ifneq ($(shell pkg-config --exists libdpdk && echo 0),0)
$(error "no installation of DPDK found")
endif

all: recursive static
.PHONY: shared static recursive
shared: build/$(APP)-shared
	ln -sf $(APP)-shared build/$(APP)
static: build/$(APP)-static
	ln -sf $(APP)-static build/$(APP)

recursive:
	$(MAKE) -C core
	$(MAKE) -C tcp

PKGCONF ?= pkg-config

# TODO O3 
PC_FILE := $(shell $(PKGCONF) --path libdpdk 2>/dev/null)
CFLAGS += $(shell $(PKGCONF) --cflags libdpdk)
CXXFLAGS+=-ggdb3 -std=c++17 -Wno-subobject-linkage -fpermissive

LDFLAGS_SHARED = $(shell $(PKGCONF) --libs libdpdk) ${LIBS}
LDFLAGS_STATIC = $(shell $(PKGCONF) --static --libs libdpdk) ${LIBS}

ifeq ($(MAKECMDGOALS),static)
# check for broken pkg-config
ifeq ($(shell echo $(LDFLAGS_STATIC) | grep 'whole-archive.*l:lib.*no-whole-archive'),)
$(warning "pkg-config output list does not contain drivers between 'whole-archive'/'no-whole-archive' flags.")
$(error "Cannot generate statically-linked binaries with this version of pkg-config")
endif
endif

CFLAGS += -DALLOW_EXPERIMENTAL_API

build/$(APP)-shared: ${LIBS} $(SRCS) Makefile $(PC_FILE) | build
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(SRCS) -Icore -o $@ $(LDFLAGS) $(LDFLAGS_SHARED)

build/$(APP)-static: ${LIBS} $(SRCS) Makefile $(PC_FILE) | build
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(SRCS) -Icore -o $@ $(LDFLAGS) $(LDFLAGS_STATIC)

build:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -f build/$(APP) build/$(APP)-static build/$(APP)-shared
	test -d build && rmdir -p build || true
