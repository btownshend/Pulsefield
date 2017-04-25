#CXX=/opt/local/bin/g++
CXX=clang++
BASEDIR=../..
UTIL=../../lib/C++Util
VPATH=.:$(UTIL)
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
CXXSRCS=calibration.cc oschandler.cc main.cc urlconfig.cc configuration.cc trackerComm.cc findTranslationRotation.cc
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

calibration.o: calibration.h lo_util.h ../../lib/C++Util/point.h
calibration.o: configuration.h ../../lib/C++Util/urlconfig.h trackerComm.h
calibration.o: ../../lib/C++Util/dbg.h findTranslationRotation.h
oschandler.o: ../../lib/C++Util/dbg.h oschandler.h lo_util.h calibration.h
oschandler.o: ../../lib/C++Util/point.h configuration.h
oschandler.o: ../../lib/C++Util/urlconfig.h
main.o: oschandler.h lo_util.h calibration.h ../../lib/C++Util/point.h
main.o: configuration.h ../../lib/C++Util/urlconfig.h ../../lib/C++Util/dbg.h
main.o: trackerComm.h
../../lib/C++Util/urlconfig.o: ../../lib/C++Util/dbg.h
../../lib/C++Util/urlconfig.o: ../../lib/C++Util/urlconfig.h
configuration.o: configuration.h ../../lib/C++Util/dbg.h
trackerComm.o: lo_util.h trackerComm.h ../../lib/C++Util/dbg.h
trackerComm.o: ../../lib/C++Util/urlconfig.h ../../lib/C++Util/point.h
findTranslationRotation.o: ../../lib/C++Util/dbg.h findTranslationRotation.h
../../lib/C++Util/dbg.o: ../../lib/C++Util/dbg.h
../../lib/C++Util/point.o: ../../lib/C++Util/point.h
../../lib/C++Util/dest.o: ../../lib/C++Util/dest.h ../../lib/C++Util/dbg.h
