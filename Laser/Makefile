#CXX=/opt/local/bin/g++
CXX=clang++
BASEDIR=../..
UTIL=../LIDAR/Util
VPATH=.:$(UTIL)
LIBLOLIBDIR=/opt/local/lib

OPENCVINC=/opt/local/include
OPENCV=-lopencv_imgproc -lopencv_core -lopencv_stitching -lopencv_calib3d
OPENCVLIBDIR=/opt/local/lib

CAIROINC=/opt/local/include/cairo
CAIROLIBDIR=/opt/local/lib
CAIROLIBES=-lcairo -lX11

NANOSVGINC=../nanosvg/src

BOOSTLIBES=-lboost_serialization-mt

ED=etherdream
EDL=$(ED)/driver/libetherdream
INCLUDES=-I$(ED)/driver/libetherdream -I$(ED)/common -I$(UTIL) -I$(CAIROINC) -I$(OPENCVINC) -I$(NANOSVGINC)
#DEBUG=-g -ggdb -Wall -D_GLIBCXX_DEBUG
DEBUG=-g  -Wall

OPTIMIZE=-O3
LDFLAGS=-L$(CAIROLIBDIR) -L$(OPENCVLIBDIR) -L$(LIBLOLIBDIR)  
LIBES=-llo $(OPENCV) $(CAIROLIBES) $(BOOSTLIBES)
# opencv generates many -Woverloaded-virtual warnings...
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES) -std=c++11 -Wno-overloaded-virtual
CFLAGS=$(DEBUG) $(OPTIMIZE)
CXXSRCS=calibration.cc configuration.cc shapeid.cc ranges.cc svg.cc oschandler.cc main.cc  laser.cc lasers.cc drawing.cc bezier.cc video.cc transform.cc etherdream_bst.cc Simplex.cpp touchosc.cc connections.cc person.cc attributes.cc groups.cc cpoint.cc music.cc conductor.cc urlconfig.cc findtargets.cc
FESRCS=dbg.cc point.cc dest.cc
#CSRCS=$(EDL)/etherdream.c
SRCS=$(CXXSRCS) $(CSRCS) $(FESRCS)

all: osclaser
	echo For debugging malloc, use: export  DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib

osclaser: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o)  $(FESRCS:.cc=.o)
	$(CXX) $(LDFLAGS)  -o $@ $^ $(LIBES)

test: test.o $(CSRCS:.c=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

beziertest: bezier.o beziertest.o point.o dbg.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

clean:
	-rm *.o

vectest: vectest.o
	$(CXX) $(LDFLAGS) -o $@ $^

vectest.o: Makefile

depend: $(SRCS)
	/opt/local/bin/makedepend -Y -I$(UTIL)  $^

# DO NOT DELETE

calibration.o: calibration.h ../LIDAR/Util/point.h touchosc.h lo_util.h
calibration.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h configuration.h
calibration.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
calibration.o: cpoint.h color.h drawing.h bezier.h attributes.h Simplex.hpp
calibration.o: ranges.h bounds.h shapeid.h
configuration.o: configuration.h ../LIDAR/Util/dbg.h
shapeid.o: shapeid.h ../LIDAR/Util/dbg.h
ranges.o: ranges.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
svg.o: drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h bezier.h color.h
svg.o: attributes.h cpoint.h Simplex.hpp touchosc.h lo_util.h
svg.o: ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h svg.h
oschandler.o: ../LIDAR/Util/dbg.h oschandler.h lo_util.h drawing.h
oschandler.o: ../LIDAR/Util/point.h bezier.h color.h attributes.h cpoint.h
oschandler.o: Simplex.hpp touchosc.h ../LIDAR/Util/urlconfig.h ranges.h
oschandler.o: bounds.h shapeid.h lasers.h laser.h etherdream_bst.h
oschandler.o: displaydevice.h transform.h configuration.h video.h
oschandler.o: connections.h person.h groups.h music.h svg.h calibration.h
main.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
main.o: ../LIDAR/Util/point.h cpoint.h color.h configuration.h touchosc.h
main.o: lo_util.h ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h drawing.h
main.o: bezier.h attributes.h Simplex.hpp ranges.h bounds.h shapeid.h video.h
main.o: oschandler.h calibration.h
laser.o: etherdream_bst.h laser.h displaydevice.h transform.h
laser.o: ../LIDAR/Util/point.h cpoint.h color.h configuration.h touchosc.h
laser.o: lo_util.h ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h drawing.h
laser.o: bezier.h attributes.h Simplex.hpp ranges.h bounds.h shapeid.h
laser.o: calibration.h
lasers.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
lasers.o: ../LIDAR/Util/point.h cpoint.h color.h configuration.h touchosc.h
lasers.o: lo_util.h ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h drawing.h
lasers.o: bezier.h attributes.h Simplex.hpp ranges.h bounds.h shapeid.h
lasers.o: connections.h person.h groups.h video.h calibration.h
drawing.o: drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h bezier.h
drawing.o: color.h attributes.h cpoint.h Simplex.hpp touchosc.h lo_util.h
drawing.o: ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h transform.h
drawing.o: etherdream_bst.h configuration.h
bezier.o: bezier.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
video.o: video.h displaydevice.h etherdream_bst.h transform.h
video.o: ../LIDAR/Util/point.h cpoint.h color.h configuration.h lasers.h
video.o: laser.h touchosc.h lo_util.h ../LIDAR/Util/dbg.h
video.o: ../LIDAR/Util/urlconfig.h drawing.h bezier.h attributes.h
video.o: Simplex.hpp ranges.h bounds.h shapeid.h calibration.h
transform.o: transform.h ../LIDAR/Util/point.h cpoint.h color.h
transform.o: etherdream_bst.h configuration.h drawing.h ../LIDAR/Util/dbg.h
transform.o: bezier.h attributes.h Simplex.hpp touchosc.h lo_util.h
transform.o: ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h
etherdream_bst.o: etherdream_bst.h ../LIDAR/Util/dbg.h
Simplex.o: Simplex.hpp
touchosc.o: touchosc.h lo_util.h ../LIDAR/Util/dbg.h
touchosc.o: ../LIDAR/Util/urlconfig.h person.h ../LIDAR/Util/point.h
touchosc.o: drawing.h bezier.h color.h attributes.h cpoint.h Simplex.hpp
touchosc.o: ranges.h bounds.h shapeid.h connections.h music.h conductor.h
touchosc.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
touchosc.o: configuration.h
connections.o: person.h ../LIDAR/Util/point.h drawing.h ../LIDAR/Util/dbg.h
connections.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
connections.o: lo_util.h ../LIDAR/Util/urlconfig.h ranges.h bounds.h
connections.o: shapeid.h connections.h conductor.h
person.o: person.h ../LIDAR/Util/point.h drawing.h ../LIDAR/Util/dbg.h
person.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
person.o: lo_util.h ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h
person.o: groups.h lasers.h laser.h etherdream_bst.h displaydevice.h
person.o: transform.h configuration.h
attributes.o: attributes.h cpoint.h color.h ../LIDAR/Util/point.h Simplex.hpp
attributes.o: touchosc.h lo_util.h ../LIDAR/Util/dbg.h
attributes.o: ../LIDAR/Util/urlconfig.h music.h svg.h drawing.h bezier.h
attributes.o: ranges.h bounds.h shapeid.h
groups.o: groups.h drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
groups.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
groups.o: lo_util.h ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h
groups.o: person.h lasers.h laser.h etherdream_bst.h displaydevice.h
groups.o: transform.h configuration.h
cpoint.o: cpoint.h color.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h bounds.h
cpoint.o: etherdream_bst.h
music.o: music.h touchosc.h lo_util.h ../LIDAR/Util/dbg.h
music.o: ../LIDAR/Util/urlconfig.h
conductor.o: conductor.h lo_util.h ../LIDAR/Util/dbg.h
../LIDAR/Util/urlconfig.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h
../LIDAR/Util/dbg.o: ../LIDAR/Util/dbg.h
../LIDAR/Util/point.o: ../LIDAR/Util/point.h
../LIDAR/Util/dest.o: ../LIDAR/Util/dest.h ../LIDAR/Util/dbg.h
