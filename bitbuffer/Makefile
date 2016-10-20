###############################################################################
#	FILE		:	Makefile
#	PROJECT		:	bitbuffer library
#	AUTHOR		:	Michael A. Uman <Michael_Uman@sigmadesigns.com>
#	DATE		:	September 18, 2013
###############################################################################

TARGET_TYPE=statlib
CPP_SOURCES=bitbuffer.cpp
ARCH?=$(shell arch)
ifdef DEBUG
	LIBNAME=libbitbufferd
else
	LIBNAME=libbitbuffer
endif

CFLAGS=-Wall -O3 -fexpensive-optimizations 
#CFLAGS=-g3 -c

LIB_DIR=../lib/$(ARCH)/

include ../../build/Build/buildsys.mk

-include $(DEPS)

