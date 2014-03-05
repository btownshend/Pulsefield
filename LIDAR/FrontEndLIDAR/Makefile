BASEDIR=../../..
LIBLO=$(BASEDIR)/OSC/liblo-0.26
SICKTOOLBOX=$(BASEDIR)/sicktoolbox/trunk/c++/drivers
SICKLIBDIR=$(SICKTOOLBOX)/lms5xx/sicklms5xx/.libs
SICKLIB=$(SICKLIBDIR)/libsicklms5xx.a 
CXX=g++
DEBUG=-g -Wall
PROFILE=
OPTIMIZE=-O3
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(LIBLO) -I$(SICKTOOLBOX)/base/src -I$(SICKTOOLBOX)/lms5xx
CFLAGS=$(DEBUG)  -O3
LDFLAGS=-L$(LIBLO) $(DEBUG) $(PROFILE)
LIBES=-llo $(SICKLIB) -g
CXXSRCS=frontend.cc dest.cc main.cc urlconfig.cc sickio.cc
CSRCS=
SRCS=$(CXXSRCS) $(CSRCS)
all: frontend

frontend: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o) $(SICKLIB)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

depend:
	makedepend -Y -- $(CXXFLAGS) -- $(SRCS)

clean:
	rm *.o

# DO NOT DELETE

frontend.o: frontend.h ../../../OSC/liblo-0.26/lo/lo.h
frontend.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
frontend.o: ../../../OSC/liblo-0.26/lo/lo_types.h
frontend.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
frontend.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
frontend.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
frontend.o: ../../../OSC/liblo-0.26/lo/lo_macros.h defs.h dest.h urlconfig.h
frontend.o: sickio.h
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
dest.o: dest.h
main.o: frontend.h ../../../OSC/liblo-0.26/lo/lo.h
main.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
main.o: ../../../OSC/liblo-0.26/lo/lo_types.h
main.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
main.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
main.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
main.o: ../../../OSC/liblo-0.26/lo/lo_macros.h defs.h dest.h
urlconfig.o: urlconfig.h
sickio.o: sickio.h
sickio.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
sickio.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
sickio.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
sickio.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
sickio.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
sickio.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
sickio.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
