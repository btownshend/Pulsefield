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


ED=etherdream
EDL=$(ED)/driver/libetherdream
INCLUDES=-I$(ED)/driver/libetherdream -I$(ED)/common -I$(UTIL) -I$(CAIROINC) -I$(OPENCVINC)
#DEBUG=-g -ggdb -Wall -D_GLIBCXX_DEBUG
DEBUG=-g  -Wall
#OPTIMIZE=-O3
LDFLAGS=-L$(CAIROLIBDIR) -L$(OPENCVLIBDIR) -L$(LIBLOLIBDIR) -ggdb
LIBES=-llo $(OPENCV) $(CAIROLIBES)
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES)
CFLAGS=$(DEBUG) $(OPTIMIZE)
CXXSRCS=oschandler.cc main.cc  laser.cc drawing.cc bezier.cc video.cc transform.cc etherdream_bst.cc
FESRCS=dbg.cc dest.cc urlconfig.cc point.cc
#CSRCS=$(EDL)/etherdream.c
SRCS=$(CXXSRCS) $(CSRCS) $(FESRCS)

all: osclaser
	echo For debugging malloc, use: export  DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib

osclaser: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o)  $(FESRCS:.cc=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

test: test.o $(CSRCS:.c=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

beziertest: bezier.o beziertest.o point.o dbg.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

clean:
	-rm *.o

vectest: vectest.o
	$(CXX) $(LDFLAGS) -o $@ $^

vectest.o: Makefile

depend:
	/opt/local/bin/makedepend -Y -- $(CXXFLAGS) -- $(SRCS)

# DO NOT DELETE

oschandler.o: ../LIDAR/Util/dbg.h oschandler.h /opt/local/include/lo/lo.h
oschandler.o: /opt/local/include/lo/lo_endian.h
oschandler.o: /opt/local/include/lo/lo_types.h
oschandler.o: /opt/local/include/lo/lo_osc_types.h
oschandler.o: /opt/local/include/lo/lo_errors.h
oschandler.o: /opt/local/include/lo/lo_lowlevel.h
oschandler.o: /opt/local/include/lo/lo_macros.h ../LIDAR/Util/dest.h
oschandler.o: drawing.h ../LIDAR/Util/point.h
oschandler.o: etherdream/driver/libetherdream/etherdream.h
oschandler.o: etherdream/common/protocol.h bezier.h laser.h displaydevice.h
oschandler.o: transform.h /opt/local/include/opencv2/core/core.hpp
oschandler.o: /opt/local/include/opencv2/core/types_c.h
oschandler.o: /opt/local/include/opencv2/core/version.hpp
oschandler.o: /opt/local/include/opencv2/core/operations.hpp
oschandler.o: /opt/local/include/opencv2/core/mat.hpp
oschandler.o: ../LIDAR/Util/urlconfig.h video.h
oschandler.o: /opt/local/include/cairo/cairo-xlib.h
oschandler.o: /opt/local/include/cairo/cairo.h
oschandler.o: /opt/local/include/cairo/cairo-version.h
oschandler.o: /opt/local/include/cairo/cairo-features.h
oschandler.o: /opt/local/include/cairo/cairo-deprecated.h
oschandler.o: /opt/local/include/X11/Xlib.h /opt/local/include/X11/X.h
oschandler.o: /opt/local/include/X11/Xfuncproto.h
oschandler.o: /opt/local/include/X11/Xosdefs.h
main.o: laser.h etherdream/driver/libetherdream/etherdream.h
main.o: etherdream/common/protocol.h displaydevice.h transform.h
main.o: /opt/local/include/opencv2/core/core.hpp
main.o: /opt/local/include/opencv2/core/types_c.h
main.o: /opt/local/include/opencv2/core/version.hpp
main.o: /opt/local/include/opencv2/core/operations.hpp
main.o: /opt/local/include/opencv2/core/mat.hpp ../LIDAR/Util/point.h
main.o: drawing.h ../LIDAR/Util/dbg.h bezier.h video.h
main.o: /opt/local/include/cairo/cairo-xlib.h
main.o: /opt/local/include/cairo/cairo.h
main.o: /opt/local/include/cairo/cairo-version.h
main.o: /opt/local/include/cairo/cairo-features.h
main.o: /opt/local/include/cairo/cairo-deprecated.h
main.o: /opt/local/include/X11/Xlib.h /opt/local/include/X11/X.h
main.o: /opt/local/include/X11/Xfuncproto.h /opt/local/include/X11/Xosdefs.h
main.o: oschandler.h /opt/local/include/lo/lo.h
main.o: /opt/local/include/lo/lo_endian.h /opt/local/include/lo/lo_types.h
main.o: /opt/local/include/lo/lo_osc_types.h
main.o: /opt/local/include/lo/lo_errors.h /opt/local/include/lo/lo_lowlevel.h
main.o: /opt/local/include/lo/lo_macros.h ../LIDAR/Util/dest.h
laser.o: etherdream/driver/libetherdream/etherdream.h
laser.o: etherdream/common/protocol.h laser.h displaydevice.h transform.h
laser.o: /opt/local/include/opencv2/core/core.hpp
laser.o: /opt/local/include/opencv2/core/types_c.h
laser.o: /opt/local/include/opencv2/core/version.hpp
laser.o: /opt/local/include/opencv2/core/operations.hpp
laser.o: /opt/local/include/opencv2/core/mat.hpp ../LIDAR/Util/point.h
laser.o: drawing.h ../LIDAR/Util/dbg.h bezier.h
drawing.o: drawing.h ../LIDAR/Util/point.h
drawing.o: etherdream/driver/libetherdream/etherdream.h
drawing.o: etherdream/common/protocol.h ../LIDAR/Util/dbg.h bezier.h
drawing.o: transform.h /opt/local/include/opencv2/core/core.hpp
drawing.o: /opt/local/include/opencv2/core/types_c.h
drawing.o: /opt/local/include/opencv2/core/version.hpp
drawing.o: /opt/local/include/opencv2/core/operations.hpp
drawing.o: /opt/local/include/opencv2/core/mat.hpp laser.h displaydevice.h
bezier.o: bezier.h ../LIDAR/Util/point.h ../LIDAR/Util/dbg.h
video.o: /opt/local/include/cairo/cairo.h
video.o: /opt/local/include/cairo/cairo-version.h
video.o: /opt/local/include/cairo/cairo-features.h
video.o: /opt/local/include/cairo/cairo-deprecated.h
video.o: /opt/local/include/cairo/cairo-xlib.h /opt/local/include/X11/Xlib.h
video.o: /opt/local/include/X11/X.h /opt/local/include/X11/Xfuncproto.h
video.o: /opt/local/include/X11/Xosdefs.h /opt/local/include/X11/Xutil.h
video.o: /opt/local/include/X11/keysym.h /opt/local/include/X11/keysymdef.h
video.o: video.h displaydevice.h etherdream/driver/libetherdream/etherdream.h
video.o: etherdream/common/protocol.h transform.h
video.o: /opt/local/include/opencv2/core/core.hpp
video.o: /opt/local/include/opencv2/core/types_c.h
video.o: /opt/local/include/opencv2/core/version.hpp
video.o: /opt/local/include/opencv2/core/operations.hpp
video.o: /opt/local/include/opencv2/core/mat.hpp ../LIDAR/Util/point.h
video.o: laser.h drawing.h ../LIDAR/Util/dbg.h bezier.h
transform.o: /opt/local/include/opencv2/imgproc/imgproc.hpp
transform.o: /opt/local/include/opencv2/core/core.hpp
transform.o: /opt/local/include/opencv2/core/types_c.h
transform.o: /opt/local/include/opencv2/core/version.hpp
transform.o: /opt/local/include/opencv2/core/operations.hpp
transform.o: /opt/local/include/opencv2/core/mat.hpp
transform.o: /opt/local/include/opencv2/imgproc/types_c.h
transform.o: /opt/local/include/opencv2/core/core_c.h transform.h
transform.o: ../LIDAR/Util/point.h
transform.o: etherdream/driver/libetherdream/etherdream.h
transform.o: etherdream/common/protocol.h drawing.h ../LIDAR/Util/dbg.h
transform.o: bezier.h
etherdream_bst.o: etherdream/driver/libetherdream/etherdream.h
etherdream_bst.o: etherdream/common/protocol.h ../LIDAR/Util/dbg.h
