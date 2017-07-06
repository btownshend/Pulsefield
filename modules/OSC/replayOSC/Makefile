LIBOSCDIR = ../libOSC
LIBOSC = ${LIBOSCDIR}/libOSC.a
DEBUG = -g
CFLAGS= $(DEBUG)  -O2 -I$(LIBOSCDIR)

replayOSC: replayOSC.o htmsocket.o ${LIBOSC}
	${CC} $(DEBUG) -o replayOSC replayOSC.o htmsocket.o  ${LIBOSC}

${LIBOSC}:
	echo "You need to go to " ${LIBOSCDIR} " and do a make."

clean:
	rm -f replayOSC *.o
