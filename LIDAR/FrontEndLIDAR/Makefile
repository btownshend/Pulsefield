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

CAIROINC=/opt/local/include/cairo
CAIROLIBDIR=/opt/local/lib
CAIROLIBES=-lcairo -lX11

CXX=g++
#DEBUG=-g -Wall -Wno-c++11-extensions
DEBUG=-g -Wall
PROFILE=
OPTIMIZE=-O3
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(LIBLO) -I$(SICKTOOLBOX)/base/src -I$(SICKTOOLBOX)/lms5xx -I$(MATLABINC) -I$(CAIROINC) -I/opt/X11/include
CFLAGS=$(DEBUG)  $(OPTIMIZE)
LDFLAGS=-L$(LIBLO) -L$(MATLABLIBDIR) $(DEBUG) $(PROFILE) -L$(CAIROLIBDIR)
LIBES=-llo $(SICKLIB) $(MATLABLIBES) $(CAIROLIBES) -g 
CXXSRCS=frontend.cc dest.cc main.cc urlconfig.cc sickio.cc background.cc world.cc snapshot.cc vis.cc person.cc target.cc likelihood.cc dbg.cc point.cc worlddraw.cc lookuptable.cc 
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
frontend.o: target.h person.h snapshot.h vis.h dbg.h
dest.o: dest.h dbg.h
main.o: frontend.h ../../../OSC/liblo-0.26/lo/lo.h
main.o: ../../../OSC/liblo-0.26/lo/lo_endian.h
main.o: ../../../OSC/liblo-0.26/lo/lo_types.h
main.o: ../../../OSC/liblo-0.26/lo/lo_osc_types.h
main.o: ../../../OSC/liblo-0.26/lo/lo_errors.h
main.o: ../../../OSC/liblo-0.26/lo/lo_lowlevel.h
main.o: ../../../OSC/liblo-0.26/lo/lo_macros.h dest.h dbg.h
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
world.o: target.h point.h person.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h dest.h
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
world.o: dbg.h parameters.h
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
snapshot.o: target.h person.h dest.h snapshot.h vis.h
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
person.o: person.h /Applications/MATLAB_R2014a.app/extern/include/mat.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h point.h
person.o: target.h vis.h sickio.h
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
person.o: parameters.h likelihood.h dbg.h normal.h lookuptable.h
target.o: target.h point.h
likelihood.o: likelihood.h target.h point.h parameters.h dbg.h
dbg.o: dbg.h
point.o: point.h
worlddraw.o: /opt/local/include/cairo/cairo.h
worlddraw.o: /opt/local/include/cairo/cairo-version.h
worlddraw.o: /opt/local/include/cairo/cairo-features.h
worlddraw.o: /opt/local/include/cairo/cairo-deprecated.h
worlddraw.o: /opt/local/include/cairo/cairo-xlib.h
worlddraw.o: /opt/X11/include/X11/Xlib.h /opt/X11/include/X11/X.h
worlddraw.o: /opt/X11/include/X11/Xfuncproto.h /opt/X11/include/X11/Xosdefs.h
worlddraw.o: world.h target.h point.h person.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h dest.h
worlddraw.o: dbg.h parameters.h
lookuptable.o: lookuptable.h normal.h dbg.h
