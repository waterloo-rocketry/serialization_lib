executable = test_main

VPATH += ../
CC = gcc
CFLAGS += -I..
CFLAGS += -lcmocka

objs  = test_main.o
objs += wrt_sdl.o

.PHONY: all clean

all: $(executable)

$(executable): $(objs)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(objs) $(executable)
