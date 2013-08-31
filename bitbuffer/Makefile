SRC=bitbuffer.cpp
OBJ=$(SRC:%.cpp=%.o)

#CFLAGS=-O2 -c
CFLAGS=-g3 -c

%.o:%.cpp
	g++ -o $*.o $(CFLAGS) $*.cpp

libbitbuffer.a : $(OBJ)
	ar rcs libbitbuffer.a $(OBJ)

bitbuffer.o : bitbuffer.cpp bitbuffer.h

clean:
	rm -rf libbitbuffer.a $(OBJ)
