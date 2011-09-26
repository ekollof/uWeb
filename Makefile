SRCS=main.c network.c web.c util.c mime.c
OBJS=main.o network.o web.o util.o mime.o

CFLAGS= -I. -O

all: webserver

webserver: $(OBJS)
	cc $(CFLAGS) -o $@ $(OBJS)
	strip $@

clean:
	-rm -f *.o *.BAK *core mon.out webserver

indent:
	for i in *.c; do indent $$i; done
