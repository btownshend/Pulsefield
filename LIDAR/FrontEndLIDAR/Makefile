BASEDIR=../../..
LIBLO=$(BASEDIR)/OSC/liblo-0.26
#MATLAB=/Applications/MATLAB_R2013a.app
MATLAB=/Applications/MATLAB_R2014a.app
MATLABINC=$(MATLAB)/extern/include
MATLABLIBDIR=$(MATLAB)/bin/maci64
MATLABLIBES=-lmx -lmat
SICKTOOLBOX=$(BASEDIR)/sicktoolbox/trunk/c++/drivers
SICKLIBDIR=$(SICKTOOLBOX)/lms5xx/sicklms5xx/.libs
SICKLIB=$(SICKLIBDIR)/libsicklms5xx.a 
CXX=g++
DEBUG=-g -Wall -Wno-c++11-extensions
PROFILE=
#OPTIMIZE=-O3
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(LIBLO) -I$(SICKTOOLBOX)/base/src -I$(SICKTOOLBOX)/lms5xx -I$(MATLABINC)
CFLAGS=$(DEBUG)  $(OPTIMIZE)
LDFLAGS=-L$(LIBLO) -L$(MATLABLIBDIR) $(DEBUG) $(PROFILE) 
LIBES=-llo $(SICKLIB) $(MATLABLIBES) -g 
CXXSRCS=frontend.cc dest.cc main.cc urlconfig.cc sickio.cc background.cc world.cc classifier.cc snapshot.cc vis.cc person.cc target.cc likelihood.cc dbg.cc point.cc
CSRCS=
SRCS=$(CXXSRCS) $(CSRCS)
all: frontend
	echo Make sure $(MATLABLIBDIR) is in DYLD_LIBRARY_PATH 

frontend: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o) $(SICKLIB)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

depend:
	/opt/local/bin/makedepend -Y -- $(CXXFLAGS) -- $(SRCS)

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
frontend.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
frontend.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
frontend.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h point.h
frontend.o: world.h classifier.h background.h person.h snapshot.h vis.h
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
sickio.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
sickio.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
sickio.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h point.h
background.o: background.h sickio.h
background.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
background.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
background.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
background.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
background.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
background.o: point.h parameters.h
world.o: world.h classifier.h background.h sickio.h
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
world.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h point.h
world.o: person.h likelihood.h vis.h
classifier.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
classifier.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
classifier.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
classifier.o: classifier.h background.h sickio.h
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
classifier.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
classifier.o: point.h parameters.h
snapshot.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
snapshot.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
snapshot.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
snapshot.o: sickio.h
snapshot.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
snapshot.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
snapshot.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
snapshot.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
snapshot.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
snapshot.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
snapshot.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
snapshot.o: point.h world.h classifier.h background.h person.h snapshot.h
snapshot.o: vis.h
vis.o: vis.h sickio.h
vis.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
vis.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
vis.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
vis.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
vis.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
vis.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
vis.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
vis.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
vis.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
vis.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h point.h
vis.o: classifier.h background.h
person.o: person.h /Applications/MATLAB_R2014a.app/extern/include/mat.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h point.h
person.o: vis.h sickio.h
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
person.o: classifier.h background.h parameters.h likelihood.h
