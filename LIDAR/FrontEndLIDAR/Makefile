BASEDIR=../../..
LIBLO=$(BASEDIR)/OSC/liblo-0.26
MATLAB=/Applications/MATLAB_R2013a.app
MATLABINC=$(MATLAB)/extern/include
MATLABLIBDIR=$(MATLAB)/bin/maci64
MATLABLIBES=-lmx -lmat
SICKTOOLBOX=$(BASEDIR)/sicktoolbox/trunk/c++/drivers
SICKLIBDIR=$(SICKTOOLBOX)/lms5xx/sicklms5xx/.libs
SICKLIB=$(SICKLIBDIR)/libsicklms5xx.a 
CXX=g++
DEBUG=-g -Wall
PROFILE=
#OPTIMIZE=-O3
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(LIBLO) -I$(SICKTOOLBOX)/base/src -I$(SICKTOOLBOX)/lms5xx -I$(MATLABINC)
CFLAGS=$(DEBUG)  $(OPTIMIZE)
LDFLAGS=-L$(LIBLO) -L$(MATLABLIBDIR) $(DEBUG) $(PROFILE) 
LIBES=-llo $(SICKLIB) $(MATLABLIBES) -g 
CXXSRCS=frontend.cc dest.cc main.cc urlconfig.cc sickio.cc background.cc tracker.cc classifier.cc snapshot.cc
CSRCS=
SRCS=$(CXXSRCS) $(CSRCS)
all: frontend
	echo Make sure $(MATLABLIBDIR) is in DYLD_LIBRARY_PATH 

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
frontend.o: tracker.h classifier.h background.h
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
background.o: background.h sickio.h
background.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
tracker.o: tracker.h classifier.h background.h sickio.h
tracker.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
tracker.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
tracker.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
tracker.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
tracker.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
tracker.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
tracker.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
classifier.o: classifier.h background.h sickio.h
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
