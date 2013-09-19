###############################################################################
#	FILE		:	Makefile
#	PROJECT		:	hevc-cut tool
#	AUTHOR		:	Michael A. Uman <Michael_Uman@sigmadesigns.com>
#	DATE		:	September 18, 2013
###############################################################################

TARGET_TYPE=exe
TARGET_EXE=hevc-cut
CPP_SOURCES=main.cpp hevcstream.cpp nalentry.cpp naltypes.cpp
EXTERN_LIBS=../bitbuffer/libbitbuffer.a

#CFLAGS=-I ../bitbuffer -g3 -Wall
CFLAGS=-I ../bitbuffer -Wall -O3 -fexpensive-optimizations 

	
include ../build/buildsys.mk

../bitbuffer/libbitbuffer.a : .PHONY
	+@make -C ../bitbuffer

cleanall: clean
	@make -C ../bitbuffer clean
