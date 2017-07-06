LIBOSCDIR = ../libOSC
LIBOSC = ${LIBOSCDIR}/libOSC.a
CFLAGS=  -O2 -I$(LIBOSCDIR)
LDFLAGS=
RECORDOBJS=recordOSC.o


recordOSC: ${RECORDOBJS}
	${CC} $(LDFLAGS) -o $@ ${RECORDOBJS}

${LIBOSC}:
	echo "You need to go to " ${LIBOSCDIR} " and do a make."

clean:
	rm -f recordOSC *.o
