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

BOOSTLIBES=-lboost_serialization-mt

ED=etherdream
EDL=$(ED)/driver/libetherdream
INCLUDES=-I$(ED)/driver/libetherdream -I$(ED)/common -I$(UTIL) -I$(CAIROINC) -I$(OPENCVINC)
#DEBUG=-g -ggdb -Wall -D_GLIBCXX_DEBUG
DEBUG=-g  -Wall
#OPTIMIZE=-O3
LDFLAGS=-L$(CAIROLIBDIR) -L$(OPENCVLIBDIR) -L$(LIBLOLIBDIR)  
LIBES=-llo $(OPENCV) $(CAIROLIBES) $(BOOSTLIBES)
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES)
CFLAGS=$(DEBUG) $(OPTIMIZE)
CXXSRCS=oschandler.cc main.cc  laser.cc lasers.cc drawing.cc bezier.cc video.cc transform.cc etherdream_bst.cc Simplex.cpp touchosc.cc connections.cc person.cc attributes.cc groups.cc cpoint.cc
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

oschandler.o: ../LIDAR/Util/dbg.h oschandler.h drawing.h
oschandler.o: ../LIDAR/Util/point.h etherdream_bst.h bezier.h color.h
oschandler.o: attributes.h Simplex.hpp touchosc.h lasers.h laser.h
oschandler.o: displaydevice.h transform.h video.h connections.h person.h
oschandler.o: groups.h
main.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
main.o: ../LIDAR/Util/point.h color.h drawing.h ../LIDAR/Util/dbg.h bezier.h
main.o: attributes.h Simplex.hpp touchosc.h video.h oschandler.h
laser.o: etherdream_bst.h laser.h displaydevice.h transform.h
laser.o: ../LIDAR/Util/point.h color.h ../LIDAR/Util/dbg.h drawing.h bezier.h
laser.o: attributes.h Simplex.hpp touchosc.h
lasers.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
lasers.o: ../LIDAR/Util/point.h color.h drawing.h ../LIDAR/Util/dbg.h
lasers.o: bezier.h attributes.h Simplex.hpp touchosc.h connections.h person.h
lasers.o: groups.h
drawing.o: drawing.h ../LIDAR/Util/point.h etherdream_bst.h
drawing.o: ../LIDAR/Util/dbg.h bezier.h color.h attributes.h Simplex.hpp
drawing.o: touchosc.h transform.h laser.h displaydevice.h
bezier.o: bezier.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
video.o: video.h displaydevice.h etherdream_bst.h transform.h
video.o: ../LIDAR/Util/point.h lasers.h laser.h color.h drawing.h
video.o: ../LIDAR/Util/dbg.h bezier.h attributes.h Simplex.hpp touchosc.h
transform.o: transform.h ../LIDAR/Util/point.h etherdream_bst.h drawing.h
transform.o: ../LIDAR/Util/dbg.h bezier.h color.h attributes.h Simplex.hpp
transform.o: touchosc.h
etherdream_bst.o: etherdream_bst.h ../LIDAR/Util/dbg.h
Simplex.o: Simplex.hpp
touchosc.o: touchosc.h ../LIDAR/Util/dbg.h person.h ../LIDAR/Util/point.h
touchosc.o: drawing.h etherdream_bst.h bezier.h color.h attributes.h
touchosc.o: Simplex.hpp connections.h
connections.o: connections.h ../LIDAR/Util/dbg.h drawing.h
connections.o: ../LIDAR/Util/point.h etherdream_bst.h bezier.h color.h
connections.o: attributes.h Simplex.hpp touchosc.h person.h
person.o: person.h ../LIDAR/Util/point.h drawing.h etherdream_bst.h
person.o: ../LIDAR/Util/dbg.h bezier.h color.h attributes.h Simplex.hpp
person.o: touchosc.h groups.h
attributes.o: attributes.h ../LIDAR/Util/point.h Simplex.hpp touchosc.h
attributes.o: ../LIDAR/Util/dbg.h
groups.o: groups.h drawing.h ../LIDAR/Util/point.h etherdream_bst.h
groups.o: ../LIDAR/Util/dbg.h bezier.h color.h attributes.h Simplex.hpp
groups.o: touchosc.h person.h
../LIDAR/Util/dbg.o: ../LIDAR/Util/dbg.h
../LIDAR/Util/point.o: ../LIDAR/Util/point.h
../LIDAR/Util/dest.o: ../LIDAR/Util/dest.h ../LIDAR/Util/dbg.h
