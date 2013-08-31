TARGET=bitstream_test
SRC=main.cpp
OBJ=$(SRC:%.cpp=%.o)
HEADERS=
LDFLAGS=-g3
LIBS=../bitbuffer/libbitbuffer.a

#CFLAGS=-O2 -c
CFLAGS=-g3 -c -I ../bitbuffer

%.o:%.cpp
	g++ -o $*.o $(CFLAGS) $*.cpp

$(TARGET) : $(OBJ) $(HEADERS)
	g++ -o $(TARGET) $(LDFLAGS) $(OBJ) $(LIBS)

clean:
	rm -rf $(TARGET) $(OBJ)

