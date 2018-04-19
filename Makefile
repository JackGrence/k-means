CC = gcc
CFLAGS = -Wall -O3
OBJECTS = main.o linklist.o display.o
LIBS = libjpeg.a

all: kmeans

debug: CFLAGS = -ggdb

debug: kmeans

windows: CC = x86_64-w64-mingw32-gcc # for windows

windows: LIBS = libjpeg_win.a # for windows

windows: kmeans

kmeans: ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} ${LIBS} -o $@

main.o: linklist.h display.h

linklist.o: linklist.h

display.o: display.h linklist.h

clean:
	-rm *.o
