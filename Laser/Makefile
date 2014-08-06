#CXX=/opt/local/bin/g++
CXX=clang++
BASEDIR=../..
UTIL=../LIDAR/Util
VPATH=.:$(UTIL)
LIBLOLIBDIR=/opt/local/lib

OPENCVINC=/opt/local/include
OPENCV=-lopencv_imgproc -lopencv_core 
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
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES)
CFLAGS=$(DEBUG) $(OPTIMIZE)
CXXSRCS=shapeid.cc ranges.cc svg.cc oschandler.cc main.cc  laser.cc lasers.cc drawing.cc bezier.cc video.cc transform.cc etherdream_bst.cc Simplex.cpp touchosc.cc connections.cc person.cc attributes.cc groups.cc cpoint.cc music.cc conductor.cc urlconfig.cc
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

shapeid.o: shapeid.h ../LIDAR/Util/dbg.h
ranges.o: ranges.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
svg.o: drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h bezier.h color.h
svg.o: attributes.h cpoint.h Simplex.hpp touchosc.h ../LIDAR/Util/urlconfig.h
svg.o: ranges.h bounds.h shapeid.h svg.h
oschandler.o: ../LIDAR/Util/dbg.h oschandler.h drawing.h
oschandler.o: ../LIDAR/Util/point.h bezier.h color.h attributes.h cpoint.h
oschandler.o: Simplex.hpp touchosc.h ../LIDAR/Util/urlconfig.h ranges.h
oschandler.o: bounds.h shapeid.h lasers.h laser.h etherdream_bst.h
oschandler.o: displaydevice.h transform.h video.h connections.h person.h
oschandler.o: groups.h music.h svg.h
main.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
main.o: ../LIDAR/Util/point.h cpoint.h color.h touchosc.h ../LIDAR/Util/dbg.h
main.o: ../LIDAR/Util/urlconfig.h drawing.h bezier.h attributes.h Simplex.hpp
main.o: ranges.h bounds.h shapeid.h video.h oschandler.h
laser.o: etherdream_bst.h laser.h displaydevice.h transform.h
laser.o: ../LIDAR/Util/point.h cpoint.h color.h touchosc.h
laser.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h drawing.h bezier.h
laser.o: attributes.h Simplex.hpp ranges.h bounds.h shapeid.h
lasers.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
lasers.o: ../LIDAR/Util/point.h cpoint.h color.h touchosc.h
lasers.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h drawing.h bezier.h
lasers.o: attributes.h Simplex.hpp ranges.h bounds.h shapeid.h connections.h
lasers.o: person.h groups.h video.h
drawing.o: drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h bezier.h
drawing.o: color.h attributes.h cpoint.h Simplex.hpp touchosc.h
drawing.o: ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h transform.h
drawing.o: etherdream_bst.h laser.h displaydevice.h
bezier.o: bezier.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
video.o: video.h displaydevice.h etherdream_bst.h transform.h
video.o: ../LIDAR/Util/point.h cpoint.h color.h lasers.h laser.h touchosc.h
video.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h drawing.h bezier.h
video.o: attributes.h Simplex.hpp ranges.h bounds.h shapeid.h
transform.o: transform.h ../LIDAR/Util/point.h cpoint.h color.h
transform.o: etherdream_bst.h drawing.h ../LIDAR/Util/dbg.h bezier.h
transform.o: attributes.h Simplex.hpp touchosc.h ../LIDAR/Util/urlconfig.h
transform.o: ranges.h bounds.h shapeid.h
etherdream_bst.o: etherdream_bst.h ../LIDAR/Util/dbg.h
Simplex.o: Simplex.hpp
touchosc.o: touchosc.h ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h person.h
touchosc.o: ../LIDAR/Util/point.h drawing.h bezier.h color.h attributes.h
touchosc.o: cpoint.h Simplex.hpp ranges.h bounds.h shapeid.h connections.h
touchosc.o: music.h conductor.h lasers.h laser.h etherdream_bst.h
touchosc.o: displaydevice.h transform.h
connections.o: person.h ../LIDAR/Util/point.h drawing.h ../LIDAR/Util/dbg.h
connections.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
connections.o: ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h
connections.o: connections.h conductor.h
person.o: person.h ../LIDAR/Util/point.h drawing.h ../LIDAR/Util/dbg.h
person.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
person.o: ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h groups.h
person.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
attributes.o: attributes.h cpoint.h color.h ../LIDAR/Util/point.h Simplex.hpp
attributes.o: touchosc.h ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h
attributes.o: music.h
groups.o: groups.h drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
groups.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
groups.o: ../LIDAR/Util/urlconfig.h ranges.h bounds.h shapeid.h person.h
groups.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
cpoint.o: cpoint.h color.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
music.o: music.h touchosc.h ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h
conductor.o: conductor.h ../LIDAR/Util/dbg.h
../LIDAR/Util/urlconfig.o: ../LIDAR/Util/dbg.h ../LIDAR/Util/urlconfig.h
../LIDAR/Util/dbg.o: ../LIDAR/Util/dbg.h
../LIDAR/Util/point.o: ../LIDAR/Util/point.h
../LIDAR/Util/dest.o: ../LIDAR/Util/dest.h ../LIDAR/Util/dbg.h
