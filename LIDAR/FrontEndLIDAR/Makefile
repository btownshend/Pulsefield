BASEDIR=../../..

UTIL=../Util
VPATH=.:$(UTIL)

#MATLAB=/Applications/MATLAB_R2013a.app
LIBLOLIBDIR=/opt/local/lib

MATLAB=/Applications/MATLAB_R2014a.app
MATLABINC=$(MATLAB)/extern/include
MATLABLIBDIR=$(MATLAB)/bin/maci64
MATLABLIBES=-lmx -lmat

SICKTOOLBOX=$(BASEDIR)/sicktoolbox/trunk/c++/drivers
SICKLIBDIR=$(SICKTOOLBOX)/lms5xx/sicklms5xx/.libs
SICKLIB=$(SICKLIBDIR)/libsicklms5xx.a 

CAIROINC=/opt/local/include/cairo
CAIROLIBDIR=/opt/local/lib
CAIROLIBES=-lcairo -lX11

OPENCVINC=-I/opt/local/include/opencv2
OPENCVLIBDIR=/opt/local/lib
OPENCVLIBES=-lopencv_ml -lopencv_video -lopencv_core

#CXX=/opt/local/bin/g++
#DEBUG=-g -ggdb -Wall -D_GLIBCXX_DEBUG
CXX=clang++
DEBUG=-g  -Wall
PROFILE=
#OPTIMIZE=-O3
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(LIBLO) -I$(SICKTOOLBOX)/base/src -I$(SICKTOOLBOX)/lms5xx -I$(MATLABINC) -I$(CAIROINC) $(OPENCVINC) -I/opt/X11/include -Wno-overloaded-virtual
CFLAGS=$(DEBUG)  $(OPTIMIZE)
LDFLAGS=-L$(LIBLOLIBDIR) -L$(MATLABLIBDIR) $(DEBUG) $(PROFILE) -L$(CAIROLIBDIR)
LIBES=-llo $(SICKLIB) $(MATLABLIBES) $(CAIROLIBES) -L$(OPENCVLIBDIR) $(OPENCVLIBES) -g 
CXXSRCS=frontend.cc dest.cc main.cc urlconfig.cc sickio.cc background.cc world.cc snapshot.cc vis.cc person.cc dbg.cc point.cc worlddraw.cc lookuptable.cc  leg.cc legstats.cc groups.cc kalmanfilter.cc
CSRCS=
SRCS=$(CXXSRCS) $(CSRCS)
all: frontend
	echo Make sure $(MATLABLIBDIR) is in DYLD_LIBRARY_PATH 

frontend: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o) $(SICKLIB)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

mixtest: mixtest.o gaussianmixture.o dbg.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)  -L$(OPENCVLIBDIR)  $(OPENCVLIBES) 

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
frontend.o: ../../../OSC/liblo-0.26/lo/lo_macros.h dest.h urlconfig.h
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
frontend.o: world.h /opt/local/include/cairo/cairo-xlib.h
frontend.o: /opt/local/include/cairo/cairo.h
frontend.o: /opt/local/include/cairo/cairo-version.h
frontend.o: /opt/local/include/cairo/cairo-features.h
frontend.o: /opt/local/include/cairo/cairo-deprecated.h
frontend.o: /opt/X11/include/X11/Xlib.h /opt/X11/include/X11/X.h
frontend.o: /opt/X11/include/X11/Xfuncproto.h /opt/X11/include/X11/Xosdefs.h
frontend.o: person.h legstats.h leg.h kalmanfilter.h
frontend.o: /opt/local/include/opencv2/video/tracking.hpp background.h
frontend.o: groups.h snapshot.h vis.h dbg.h parameters.h
dest.o: dest.h dbg.h
main.o: frontend.h ../../../OSC/liblo-0.26/lo/lo.h
main.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
main.o: ../../../OSC/liblo-0.26/lo/lo_types.h
main.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
main.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
main.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
main.o: ../../../OSC/liblo-0.26/lo/lo_macros.h dest.h dbg.h parameters.h
urlconfig.o: dbg.h urlconfig.h
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
sickio.o: dbg.h
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
background.o: point.h parameters.h dbg.h
world.o: ../../../OSC/liblo-0.26/lo/lo.h
world.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
world.o: ../../../OSC/liblo-0.26/lo/lo_types.h
world.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
world.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
world.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
world.o: ../../../OSC/liblo-0.26/lo/lo_macros.h world.h
world.o: /opt/local/include/cairo/cairo-xlib.h
world.o: /opt/local/include/cairo/cairo.h
world.o: /opt/local/include/cairo/cairo-version.h
world.o: /opt/local/include/cairo/cairo-features.h
world.o: /opt/local/include/cairo/cairo-deprecated.h
world.o: /opt/X11/include/X11/Xlib.h /opt/X11/include/X11/X.h
world.o: /opt/X11/include/X11/Xfuncproto.h /opt/X11/include/X11/Xosdefs.h
world.o: person.h /Applications/MATLAB_R2014a.app/extern/include/mat.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h point.h
world.o: legstats.h leg.h kalmanfilter.h
world.o: /opt/local/include/opencv2/video/tracking.hpp dest.h background.h
world.o: sickio.h
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
world.o: groups.h vis.h dbg.h parameters.h
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
snapshot.o: point.h world.h /opt/local/include/cairo/cairo-xlib.h
snapshot.o: /opt/local/include/cairo/cairo.h
snapshot.o: /opt/local/include/cairo/cairo-version.h
snapshot.o: /opt/local/include/cairo/cairo-features.h
snapshot.o: /opt/local/include/cairo/cairo-deprecated.h
snapshot.o: /opt/X11/include/X11/Xlib.h /opt/X11/include/X11/X.h
snapshot.o: /opt/X11/include/X11/Xfuncproto.h /opt/X11/include/X11/Xosdefs.h
snapshot.o: person.h ../../../OSC/liblo-0.26/lo/lo.h
snapshot.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
snapshot.o: ../../../OSC/liblo-0.26/lo/lo_types.h
snapshot.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
snapshot.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
snapshot.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
snapshot.o: ../../../OSC/liblo-0.26/lo/lo_macros.h legstats.h leg.h
snapshot.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
snapshot.o: dest.h background.h groups.h snapshot.h vis.h
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
vis.o: parameters.h
person.o: person.h /Applications/MATLAB_R2014a.app/extern/include/mat.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
person.o: ../../../OSC/liblo-0.26/lo/lo.h
person.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
person.o: ../../../OSC/liblo-0.26/lo/lo_types.h
person.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
person.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
person.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
person.o: ../../../OSC/liblo-0.26/lo/lo_macros.h point.h legstats.h leg.h
person.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp vis.h
person.o: sickio.h
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
person.o: parameters.h dbg.h normal.h lookuptable.h groups.h
dbg.o: dbg.h
point.o: point.h
worlddraw.o: /opt/local/include/cairo/cairo.h
worlddraw.o: /opt/local/include/cairo/cairo-version.h
worlddraw.o: /opt/local/include/cairo/cairo-features.h
worlddraw.o: /opt/local/include/cairo/cairo-deprecated.h
worlddraw.o: /opt/local/include/cairo/cairo-xlib.h
worlddraw.o: /opt/X11/include/X11/Xlib.h /opt/X11/include/X11/X.h
worlddraw.o: /opt/X11/include/X11/Xfuncproto.h /opt/X11/include/X11/Xosdefs.h
worlddraw.o: world.h person.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
worlddraw.o: ../../../OSC/liblo-0.26/lo/lo.h
worlddraw.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
worlddraw.o: ../../../OSC/liblo-0.26/lo/lo_types.h
worlddraw.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
worlddraw.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
worlddraw.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
worlddraw.o: ../../../OSC/liblo-0.26/lo/lo_macros.h point.h legstats.h leg.h
worlddraw.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
worlddraw.o: dest.h background.h sickio.h
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
worlddraw.o: groups.h dbg.h parameters.h
lookuptable.o: lookuptable.h normal.h dbg.h parameters.h
leg.o: ../../../OSC/liblo-0.26/lo/lo.h ../../../OSC/liblo-0.26/lo/lo_endian.h
leg.o: ../../../OSC/liblo-0.26/lo/lo_types.h
leg.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
leg.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
leg.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
leg.o: ../../../OSC/liblo-0.26/lo/lo_macros.h legstats.h leg.h point.h
leg.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
leg.o: parameters.h dbg.h normal.h vis.h sickio.h
leg.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
leg.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
leg.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
leg.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
leg.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
leg.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
leg.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
leg.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
leg.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
leg.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
leg.o: lookuptable.h
legstats.o: person.h /Applications/MATLAB_R2014a.app/extern/include/mat.h
legstats.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
legstats.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
legstats.o: ../../../OSC/liblo-0.26/lo/lo.h
legstats.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
legstats.o: ../../../OSC/liblo-0.26/lo/lo_types.h
legstats.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
legstats.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
legstats.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
legstats.o: ../../../OSC/liblo-0.26/lo/lo_macros.h point.h legstats.h leg.h
legstats.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
legstats.o: parameters.h dbg.h
groups.o: groups.h ../../../OSC/liblo-0.26/lo/lo.h
groups.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
groups.o: ../../../OSC/liblo-0.26/lo/lo_types.h
groups.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
groups.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
groups.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
groups.o: ../../../OSC/liblo-0.26/lo/lo_macros.h point.h person.h
groups.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
groups.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
groups.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
groups.o: legstats.h leg.h kalmanfilter.h
groups.o: /opt/local/include/opencv2/video/tracking.hpp dbg.h parameters.h
kalmanfilter.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
kalmanfilter.o: point.h
