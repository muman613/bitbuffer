###############################################################################
#	FILE		:	Makefile
#	PROJECT		:	bitstream test app
#	AUTHOR		:	Michael A. Uman <Michael_Uman@sigmadesigns.com>
#	DATE		:	October 20, 2016
###############################################################################

TARGET_TYPE=exe
TARGET_EXE=bitstream_test
CPP_SOURCES=main.cpp
ARCH?=$(shell arch)

BITBUFFER_INC=-I ../bitbuffer
ifdef DEBUG
	BITBUFFER_LIBS=../lib/$(ARCH)/libbitbufferd.a
else
	BITBUFFER_LIBS=../lib/$(ARCH)/libbitbuffer.a
endif

CFLAGS=-Wall -O3 -fexpensive-optimizations $(BITBUFFER_INC)
LDFLAGS=$(BITBUFFER_LIBS)

include ../../build/Build/buildsys.mk

-include $(DEPS)

