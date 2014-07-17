BASEDIR=../../..

UTIL=../Util
VPATH=.:$(UTIL)

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

OPENCVINC=-I/opt/local/include
OPENCVLIBDIR=/opt/local/lib
OPENCVLIBES=-lopencv_ml -lopencv_video -lopencv_core

#CXX=/opt/local/bin/g++
#DEBUG=-g -ggdb -Wall -D_GLIBCXX_DEBUG
CXX=clang++
DEBUG=-g  -Wall
PROFILE=
OPTIMIZE=-Ofast
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(UTIL) -I$(SICKTOOLBOX)/lms5xx -I$(SICKTOOLBOX)/base/src -I$(MATLABINC) -I$(CAIROINC) $(OPENCVINC) -I/opt/X11/include -Wno-overloaded-virtual
CFLAGS=$(DEBUG)  $(OPTIMIZE)
LDFLAGS=-L$(LIBLOLIBDIR) -L$(MATLABLIBDIR) $(DEBUG) $(PROFILE) -L$(CAIROLIBDIR)
LIBES=-llo $(SICKLIB) $(MATLABLIBES) $(CAIROLIBES) -L$(OPENCVLIBDIR) $(OPENCVLIBES) -g 
CXXSRCS=frontend.cc handlers.cc dest.cc main.cc urlconfig.cc sickio.cc background.cc world.cc snapshot.cc vis.cc person.cc dbg.cc point.cc worlddraw.cc lookuptable.cc  leg.cc legstats.cc groups.cc kalmanfilter.cc
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

frontend.o: frontend.h /opt/local/include/lo/lo.h
frontend.o: /opt/local/include/lo/lo_endian.h
frontend.o: /opt/local/include/lo/lo_types.h
frontend.o: /opt/local/include/lo/lo_osc_types.h
frontend.o: /opt/local/include/lo/lo_errors.h
frontend.o: /opt/local/include/lo/lo_lowlevel.h
frontend.o: /opt/local/include/lo/lo_macros.h ../Util/dest.h
frontend.o: ../Util/urlconfig.h sickio.h
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
frontend.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
frontend.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
frontend.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
frontend.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
frontend.o: ../Util/point.h ../Util/dbg.h world.h
frontend.o: /opt/local/include/cairo/cairo-xlib.h
frontend.o: /opt/local/include/cairo/cairo.h
frontend.o: /opt/local/include/cairo/cairo-version.h
frontend.o: /opt/local/include/cairo/cairo-features.h
frontend.o: /opt/local/include/cairo/cairo-deprecated.h
frontend.o: /opt/local/include/X11/Xlib.h /opt/local/include/X11/X.h
frontend.o: /opt/local/include/X11/Xfuncproto.h
frontend.o: /opt/local/include/X11/Xosdefs.h person.h legstats.h leg.h
frontend.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
frontend.o: /opt/local/include/opencv2/core/core.hpp
frontend.o: /opt/local/include/opencv2/core/types_c.h
frontend.o: /opt/local/include/opencv2/core/version.hpp
frontend.o: /opt/local/include/opencv2/core/operations.hpp
frontend.o: /opt/local/include/opencv2/core/mat.hpp
frontend.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
frontend.o: /opt/local/include/opencv2/imgproc/types_c.h
frontend.o: /opt/local/include/opencv2/core/core_c.h background.h groups.h
frontend.o: snapshot.h vis.h parameters.h
main.o: frontend.h /opt/local/include/lo/lo.h
main.o: /opt/local/include/lo/lo_endian.h /opt/local/include/lo/lo_types.h
main.o: /opt/local/include/lo/lo_osc_types.h
main.o: /opt/local/include/lo/lo_errors.h /opt/local/include/lo/lo_lowlevel.h
main.o: /opt/local/include/lo/lo_macros.h ../Util/dest.h ../Util/dbg.h
main.o: parameters.h
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
sickio.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
sickio.o: ../Util/point.h ../Util/dbg.h
background.o: background.h /opt/local/include/lo/lo.h
background.o: /opt/local/include/lo/lo_endian.h
background.o: /opt/local/include/lo/lo_types.h
background.o: /opt/local/include/lo/lo_osc_types.h
background.o: /opt/local/include/lo/lo_errors.h
background.o: /opt/local/include/lo/lo_lowlevel.h
background.o: /opt/local/include/lo/lo_macros.h sickio.h
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
background.o: ../Util/point.h ../Util/dbg.h parameters.h
world.o: /opt/local/include/lo/lo.h /opt/local/include/lo/lo_endian.h
world.o: /opt/local/include/lo/lo_types.h
world.o: /opt/local/include/lo/lo_osc_types.h
world.o: /opt/local/include/lo/lo_errors.h
world.o: /opt/local/include/lo/lo_lowlevel.h
world.o: /opt/local/include/lo/lo_macros.h world.h
world.o: /opt/local/include/cairo/cairo-xlib.h
world.o: /opt/local/include/cairo/cairo.h
world.o: /opt/local/include/cairo/cairo-version.h
world.o: /opt/local/include/cairo/cairo-features.h
world.o: /opt/local/include/cairo/cairo-deprecated.h
world.o: /opt/local/include/X11/Xlib.h /opt/local/include/X11/X.h
world.o: /opt/local/include/X11/Xfuncproto.h /opt/local/include/X11/Xosdefs.h
world.o: person.h /Applications/MATLAB_R2014a.app/extern/include/mat.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
world.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
world.o: ../Util/point.h legstats.h leg.h kalmanfilter.h
world.o: /opt/local/include/opencv2/video/tracking.hpp
world.o: /opt/local/include/opencv2/core/core.hpp
world.o: /opt/local/include/opencv2/core/types_c.h
world.o: /opt/local/include/opencv2/core/version.hpp
world.o: /opt/local/include/opencv2/core/operations.hpp
world.o: /opt/local/include/opencv2/core/mat.hpp
world.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
world.o: /opt/local/include/opencv2/imgproc/types_c.h
world.o: /opt/local/include/opencv2/core/core_c.h ../Util/dest.h background.h
world.o: sickio.h
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
world.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
world.o: ../Util/dbg.h groups.h vis.h parameters.h
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
snapshot.o: ../Util/point.h ../Util/dbg.h world.h
snapshot.o: /opt/local/include/cairo/cairo-xlib.h
snapshot.o: /opt/local/include/cairo/cairo.h
snapshot.o: /opt/local/include/cairo/cairo-version.h
snapshot.o: /opt/local/include/cairo/cairo-features.h
snapshot.o: /opt/local/include/cairo/cairo-deprecated.h
snapshot.o: /opt/local/include/X11/Xlib.h /opt/local/include/X11/X.h
snapshot.o: /opt/local/include/X11/Xfuncproto.h
snapshot.o: /opt/local/include/X11/Xosdefs.h person.h
snapshot.o: /opt/local/include/lo/lo.h /opt/local/include/lo/lo_endian.h
snapshot.o: /opt/local/include/lo/lo_types.h
snapshot.o: /opt/local/include/lo/lo_osc_types.h
snapshot.o: /opt/local/include/lo/lo_errors.h
snapshot.o: /opt/local/include/lo/lo_lowlevel.h
snapshot.o: /opt/local/include/lo/lo_macros.h legstats.h leg.h kalmanfilter.h
snapshot.o: /opt/local/include/opencv2/video/tracking.hpp
snapshot.o: /opt/local/include/opencv2/core/core.hpp
snapshot.o: /opt/local/include/opencv2/core/types_c.h
snapshot.o: /opt/local/include/opencv2/core/version.hpp
snapshot.o: /opt/local/include/opencv2/core/operations.hpp
snapshot.o: /opt/local/include/opencv2/core/mat.hpp
snapshot.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
snapshot.o: /opt/local/include/opencv2/imgproc/types_c.h
snapshot.o: /opt/local/include/opencv2/core/core_c.h ../Util/dest.h
snapshot.o: background.h groups.h snapshot.h vis.h
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
vis.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
vis.o: ../Util/point.h ../Util/dbg.h parameters.h
person.o: person.h /Applications/MATLAB_R2014a.app/extern/include/mat.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
person.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
person.o: /opt/local/include/lo/lo.h /opt/local/include/lo/lo_endian.h
person.o: /opt/local/include/lo/lo_types.h
person.o: /opt/local/include/lo/lo_osc_types.h
person.o: /opt/local/include/lo/lo_errors.h
person.o: /opt/local/include/lo/lo_lowlevel.h
person.o: /opt/local/include/lo/lo_macros.h ../Util/point.h legstats.h leg.h
person.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
person.o: /opt/local/include/opencv2/core/core.hpp
person.o: /opt/local/include/opencv2/core/types_c.h
person.o: /opt/local/include/opencv2/core/version.hpp
person.o: /opt/local/include/opencv2/core/operations.hpp
person.o: /opt/local/include/opencv2/core/mat.hpp
person.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
person.o: /opt/local/include/opencv2/imgproc/types_c.h
person.o: /opt/local/include/opencv2/core/core_c.h vis.h sickio.h
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
person.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
person.o: ../Util/dbg.h parameters.h normal.h lookuptable.h groups.h
worlddraw.o: /opt/local/include/cairo/cairo.h
worlddraw.o: /opt/local/include/cairo/cairo-version.h
worlddraw.o: /opt/local/include/cairo/cairo-features.h
worlddraw.o: /opt/local/include/cairo/cairo-deprecated.h
worlddraw.o: /opt/local/include/cairo/cairo-xlib.h
worlddraw.o: /opt/local/include/X11/Xlib.h /opt/local/include/X11/X.h
worlddraw.o: /opt/local/include/X11/Xfuncproto.h
worlddraw.o: /opt/local/include/X11/Xosdefs.h world.h person.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
worlddraw.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
worlddraw.o: /opt/local/include/lo/lo.h /opt/local/include/lo/lo_endian.h
worlddraw.o: /opt/local/include/lo/lo_types.h
worlddraw.o: /opt/local/include/lo/lo_osc_types.h
worlddraw.o: /opt/local/include/lo/lo_errors.h
worlddraw.o: /opt/local/include/lo/lo_lowlevel.h
worlddraw.o: /opt/local/include/lo/lo_macros.h ../Util/point.h legstats.h
worlddraw.o: leg.h kalmanfilter.h
worlddraw.o: /opt/local/include/opencv2/video/tracking.hpp
worlddraw.o: /opt/local/include/opencv2/core/core.hpp
worlddraw.o: /opt/local/include/opencv2/core/types_c.h
worlddraw.o: /opt/local/include/opencv2/core/version.hpp
worlddraw.o: /opt/local/include/opencv2/core/operations.hpp
worlddraw.o: /opt/local/include/opencv2/core/mat.hpp
worlddraw.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
worlddraw.o: /opt/local/include/opencv2/imgproc/types_c.h
worlddraw.o: /opt/local/include/opencv2/core/core_c.h ../Util/dest.h
worlddraw.o: background.h sickio.h
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xx.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickLIDAR.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickException.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxBufferMonitor.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/lms5xx/sicklms5xx/SickLMS5xxMessage.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickMessage.hh
worlddraw.o: ../../../sicktoolbox/trunk/c++/drivers/base/src/SickBufferMonitor.hh
worlddraw.o: ../Util/dbg.h groups.h parameters.h vis.h
lookuptable.o: lookuptable.h normal.h ../Util/dbg.h parameters.h
leg.o: /opt/local/include/lo/lo.h /opt/local/include/lo/lo_endian.h
leg.o: /opt/local/include/lo/lo_types.h /opt/local/include/lo/lo_osc_types.h
leg.o: /opt/local/include/lo/lo_errors.h /opt/local/include/lo/lo_lowlevel.h
leg.o: /opt/local/include/lo/lo_macros.h legstats.h leg.h ../Util/point.h
leg.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
leg.o: /opt/local/include/opencv2/core/core.hpp
leg.o: /opt/local/include/opencv2/core/types_c.h
leg.o: /opt/local/include/opencv2/core/version.hpp
leg.o: /opt/local/include/opencv2/core/operations.hpp
leg.o: /opt/local/include/opencv2/core/mat.hpp
leg.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
leg.o: /opt/local/include/opencv2/imgproc/types_c.h
leg.o: /opt/local/include/opencv2/core/core_c.h parameters.h ../Util/dbg.h
leg.o: normal.h vis.h sickio.h
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
legstats.o: /opt/local/include/lo/lo.h /opt/local/include/lo/lo_endian.h
legstats.o: /opt/local/include/lo/lo_types.h
legstats.o: /opt/local/include/lo/lo_osc_types.h
legstats.o: /opt/local/include/lo/lo_errors.h
legstats.o: /opt/local/include/lo/lo_lowlevel.h
legstats.o: /opt/local/include/lo/lo_macros.h ../Util/point.h legstats.h
legstats.o: leg.h kalmanfilter.h
legstats.o: /opt/local/include/opencv2/video/tracking.hpp
legstats.o: /opt/local/include/opencv2/core/core.hpp
legstats.o: /opt/local/include/opencv2/core/types_c.h
legstats.o: /opt/local/include/opencv2/core/version.hpp
legstats.o: /opt/local/include/opencv2/core/operations.hpp
legstats.o: /opt/local/include/opencv2/core/mat.hpp
legstats.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
legstats.o: /opt/local/include/opencv2/imgproc/types_c.h
legstats.o: /opt/local/include/opencv2/core/core_c.h parameters.h
legstats.o: ../Util/dbg.h
groups.o: groups.h /opt/local/include/lo/lo.h
groups.o: /opt/local/include/lo/lo_endian.h /opt/local/include/lo/lo_types.h
groups.o: /opt/local/include/lo/lo_osc_types.h
groups.o: /opt/local/include/lo/lo_errors.h
groups.o: /opt/local/include/lo/lo_lowlevel.h
groups.o: /opt/local/include/lo/lo_macros.h ../Util/point.h person.h
groups.o: /Applications/MATLAB_R2014a.app/extern/include/mat.h
groups.o: /Applications/MATLAB_R2014a.app/extern/include/matrix.h
groups.o: /Applications/MATLAB_R2014a.app/extern/include/tmwtypes.h
groups.o: legstats.h leg.h kalmanfilter.h
groups.o: /opt/local/include/opencv2/video/tracking.hpp
groups.o: /opt/local/include/opencv2/core/core.hpp
groups.o: /opt/local/include/opencv2/core/types_c.h
groups.o: /opt/local/include/opencv2/core/version.hpp
groups.o: /opt/local/include/opencv2/core/operations.hpp
groups.o: /opt/local/include/opencv2/core/mat.hpp
groups.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
groups.o: /opt/local/include/opencv2/imgproc/types_c.h
groups.o: /opt/local/include/opencv2/core/core_c.h ../Util/dbg.h parameters.h
kalmanfilter.o: kalmanfilter.h /opt/local/include/opencv2/video/tracking.hpp
kalmanfilter.o: /opt/local/include/opencv2/core/core.hpp
kalmanfilter.o: /opt/local/include/opencv2/core/types_c.h
kalmanfilter.o: /opt/local/include/opencv2/core/version.hpp
kalmanfilter.o: /opt/local/include/opencv2/core/operations.hpp
kalmanfilter.o: /opt/local/include/opencv2/core/mat.hpp
kalmanfilter.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
kalmanfilter.o: /opt/local/include/opencv2/imgproc/types_c.h
kalmanfilter.o: /opt/local/include/opencv2/core/core_c.h ../Util/point.h
