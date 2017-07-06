LIBOSCDIR = ../libOSC
LIBOSC = ${LIBOSCDIR}/libOSC.a
DEBUG = -g
CFLAGS= $(DEBUG)  -O2 -I$(LIBOSCDIR)
LDFLAGS=$(DEBUG)
sendOSC: sendOSC.o htmsocket.o ${LIBOSC}
	${CC} $(LDFLAGS) -o sendOSC sendOSC.o htmsocket.o  ${LIBOSC}

${LIBOSC}:
	echo "You need to go to " ${LIBOSCDIR} " and do a make."

clean:
	rm -f sendOSC *.o
