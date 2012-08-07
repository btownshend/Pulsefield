LIBLO=../../OSC/liblo-0.26
CXX=g++
DEBUG=-g -Wall
PROFILE=
OPTIMIZE=
CXXFLAGS=$(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(LIBLO) 
CFLAGS=$(CXXFLAGS)
LDFLAGS=-L$(LIBLO) $(DEBUG) $(PROFILE)
LIBES=-llo -g
CXXSRCS=frontend.cc camio.cc frame.cc  visible.cc dest.cc main.cc urlconfig.cc
CSRCS=nanojpeg.c
SRCS=$(CXXSRCS) $(CSRCS)

frontend: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

depend:
	makedepend -Y -- $(CXXFLAGS) -- $(SRCS)

clean:
	rm *.o

# DO NOT DELETE

frontend.o: camio.h frame.h defs.h visible.h frontend.h
frontend.o: ../../OSC/liblo-0.26/lo/lo.h ../../OSC/liblo-0.26/lo/lo_endian.h
frontend.o: ../../OSC/liblo-0.26/lo/lo_types.h
frontend.o: ../../OSC/liblo-0.26/lo/lo_osc_types.h
frontend.o: ../../OSC/liblo-0.26/lo/lo_errors.h
frontend.o: ../../OSC/liblo-0.26/lo/lo_lowlevel.h
frontend.o: ../../OSC/liblo-0.26/lo/lo_macros.h dest.h
camio.o: camio.h frame.h defs.h
frame.o: frame.h defs.h nanojpeg.h
visible.o: visible.h frame.h defs.h
dest.o: dest.h
main.o: frontend.h ../../OSC/liblo-0.26/lo/lo.h
main.o: ../../OSC/liblo-0.26/lo/lo_endian.h
main.o: ../../OSC/liblo-0.26/lo/lo_types.h
main.o: ../../OSC/liblo-0.26/lo/lo_osc_types.h
main.o: ../../OSC/liblo-0.26/lo/lo_errors.h
main.o: ../../OSC/liblo-0.26/lo/lo_lowlevel.h
main.o: ../../OSC/liblo-0.26/lo/lo_macros.h defs.h dest.h
