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
CXXSRCS=oschandler.cc main.cc  laser.cc lasers.cc drawing.cc bezier.cc video.cc transform.cc etherdream_bst.cc Simplex.cpp touchosc.cc connections.cc person.cc attributes.cc groups.cc cpoint.cc music.cc conductor.cc
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
oschandler.o: ../LIDAR/Util/point.h bezier.h color.h attributes.h cpoint.h
oschandler.o: Simplex.hpp touchosc.h lasers.h laser.h etherdream_bst.h
oschandler.o: displaydevice.h transform.h video.h connections.h person.h
oschandler.o: groups.h music.h
main.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
main.o: ../LIDAR/Util/point.h cpoint.h color.h touchosc.h ../LIDAR/Util/dbg.h
main.o: drawing.h bezier.h attributes.h Simplex.hpp video.h oschandler.h
laser.o: etherdream_bst.h laser.h displaydevice.h transform.h
laser.o: ../LIDAR/Util/point.h cpoint.h color.h touchosc.h
laser.o: ../LIDAR/Util/dbg.h drawing.h bezier.h attributes.h Simplex.hpp
lasers.o: lasers.h laser.h etherdream_bst.h displaydevice.h transform.h
lasers.o: ../LIDAR/Util/point.h cpoint.h color.h touchosc.h
lasers.o: ../LIDAR/Util/dbg.h drawing.h bezier.h attributes.h Simplex.hpp
lasers.o: connections.h person.h groups.h
drawing.o: drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h bezier.h
drawing.o: color.h attributes.h cpoint.h Simplex.hpp touchosc.h transform.h
drawing.o: etherdream_bst.h laser.h displaydevice.h
bezier.o: bezier.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
video.o: video.h displaydevice.h etherdream_bst.h transform.h
video.o: ../LIDAR/Util/point.h cpoint.h color.h lasers.h laser.h touchosc.h
video.o: ../LIDAR/Util/dbg.h drawing.h bezier.h attributes.h Simplex.hpp
transform.o: transform.h ../LIDAR/Util/point.h cpoint.h color.h
transform.o: etherdream_bst.h drawing.h ../LIDAR/Util/dbg.h bezier.h
transform.o: attributes.h Simplex.hpp touchosc.h
etherdream_bst.o: etherdream_bst.h ../LIDAR/Util/dbg.h
Simplex.o: Simplex.hpp
touchosc.o: touchosc.h ../LIDAR/Util/dbg.h person.h ../LIDAR/Util/point.h
touchosc.o: drawing.h bezier.h color.h attributes.h cpoint.h Simplex.hpp
touchosc.o: connections.h music.h conductor.h
connections.o: person.h ../LIDAR/Util/point.h drawing.h ../LIDAR/Util/dbg.h
connections.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
connections.o: connections.h conductor.h
person.o: person.h ../LIDAR/Util/point.h drawing.h ../LIDAR/Util/dbg.h
person.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
person.o: groups.h
attributes.o: attributes.h cpoint.h color.h ../LIDAR/Util/point.h Simplex.hpp
attributes.o: touchosc.h ../LIDAR/Util/dbg.h music.h
groups.o: groups.h drawing.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
groups.o: bezier.h color.h attributes.h cpoint.h Simplex.hpp touchosc.h
groups.o: person.h
cpoint.o: cpoint.h color.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
music.o: music.h touchosc.h ../LIDAR/Util/dbg.h
conductor.o: conductor.h ../LIDAR/Util/dbg.h
../LIDAR/Util/dbg.o: ../LIDAR/Util/dbg.h
../LIDAR/Util/point.o: ../LIDAR/Util/point.h
../LIDAR/Util/dest.o: ../LIDAR/Util/dest.h ../LIDAR/Util/dbg.h
