#CXX=/opt/local/bin/g++
CXX=clang++
BASEDIR=../..
UTIL=../LIDAR/Util
VPATH=.:$(UTIL):../Laser
LIBLOLIBDIR=/opt/local/lib

OPENCVINC=/opt/local/include
OPENCV=-lopencv_imgproc -lopencv_core -lopencv_stitching -lopencv_calib3d
OPENCVLIBDIR=/opt/local/lib

BOOSTLIBES=-lboost_serialization-mt

INCLUDES=-I$(UTIL) -I$(OPENCVINC)
#DEBUG=-g -ggdb -Wall -D_GLIBCXX_DEBUG
DEBUG=-g  -Wall

OPTIMIZE=-O3
LDFLAGS=-L$(OPENCVLIBDIR) -L$(LIBLOLIBDIR)  
LIBES=-llo $(OPENCV) $(BOOSTLIBES)
# opencv generates many -Woverloaded-virtual warnings...
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES) -std=c++11 -Wno-overloaded-virtual  -Wno-unused-local-typedef
CFLAGS=$(DEBUG) $(OPTIMIZE)
CXXSRCS=calibration.cc oschandler.cc main.cc touchosc.cc urlconfig.cc configuration.cc trackerComm.cc
FESRCS=dbg.cc point.cc dest.cc
SRCS=$(CXXSRCS) $(CSRCS) $(FESRCS)

all: calibrator
	echo For debugging malloc, use: export  DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib

calibrator: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o)  $(FESRCS:.cc=.o)
	$(CXX) $(LDFLAGS)  -o $@ $^ $(LIBES)

clean:
	-rm *.o


depend: $(SRCS)
	/opt/local/bin/makedepend -Y -I$(UTIL)  $^

# DO NOT DELETE

calibration.o: calibration.h ../LIDAR/Util/point.h touchosc.h lo_util.h
calibration.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h configuration.h
calibration.o: trackerComm.h
oschandler.o: ../LIDAR/Util/dbg.h oschandler.h lo_util.h touchosc.h
oschandler.o: ../LIDAR/Util/urlconfig.h calibration.h ../LIDAR/Util/point.h
oschandler.o: configuration.h
main.o: oschandler.h lo_util.h calibration.h ../LIDAR/Util/point.h touchosc.h
main.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h configuration.h
touchosc.o: touchosc.h lo_util.h ../LIDAR/Util/dbg.h
touchosc.o: ../LIDAR/Util/urlconfig.h
../LIDAR/Util/urlconfig.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h
../Laser/configuration.o: configuration.h ../LIDAR/Util/dbg.h
trackerComm.o: touchosc.h lo_util.h ../LIDAR/Util/dbg.h
trackerComm.o: ../LIDAR/Util/urlconfig.h trackerComm.h
../LIDAR/Util/dbg.o: ../LIDAR/Util/dbg.h
../LIDAR/Util/point.o: ../LIDAR/Util/point.h
../LIDAR/Util/dest.o: ../LIDAR/Util/dest.h ../LIDAR/Util/dbg.h
