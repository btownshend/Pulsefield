CXX=/opt/local/bin/g++
BASEDIR=../..
UTIL=../LIDAR/FrontEndLIDAR    # For dbg
VPATH=.:$(UTIL)
#LIBLO=$(BASEDIR)/OSC/liblo-0.26

OPENCV=-lopencv_imgproc -lopencv_core 

CAIROINC=/opt/local/include/cairo
CAIROLIBDIR=/opt/local/lib
CAIROLIBES=-lcairo -lX11


ED=etherdream
EDL=$(ED)/driver/libetherdream
INCLUDES=-I$(ED)/driver/libetherdream -I$(ED)/common -I$(UTIL) -I$(CAIROINC)
DEBUG=-ggdb -Wall
LDFLAGS=-L$(CAIROLIBDIR) -ggdb
LIBES=-llo $(OPENCV) $(CAIROLIBES)
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES)
CFLAGS=$(DEBUG) $(OPTIMIZE)
CXXSRCS=oschandler.cc main.cc  dbg.cc dest.cc urlconfig.cc laser.cc point.cc drawing.cc bezier.cc video.cc transform.cc etherdream_bst.cc
#CSRCS=$(EDL)/etherdream.c
SRCS=$(CXXSRCS) $(CSRCS)

all: osclaser

osclaser: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

test: test.o $(CSRCS:.c=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

beziertest: bezier.o beziertest.o point.o dbg.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

clean:
	-rm *.o

depend:
	/opt/local/bin/makedepend -Y -- $(CXXFLAGS) -- $(SRCS)

# DO NOT DELETE

oschandler.o: ../LIDAR/FrontEndLIDAR/dbg.h oschandler.h
oschandler.o: ../LIDAR/FrontEndLIDAR/dest.h drawing.h
oschandler.o: ../LIDAR/FrontEndLIDAR/point.h
oschandler.o: etherdream/driver/libetherdream/etherdream.h
oschandler.o: etherdream/common/protocol.h bezier.h laser.h displaydevice.h
oschandler.o: transform.h ../LIDAR/FrontEndLIDAR/urlconfig.h video.h
oschandler.o: /opt/local/include/cairo/cairo-xlib.h
oschandler.o: /opt/local/include/cairo/cairo.h
oschandler.o: /opt/local/include/cairo/cairo-version.h
oschandler.o: /opt/local/include/cairo/cairo-features.h
oschandler.o: /opt/local/include/cairo/cairo-deprecated.h
main.o: laser.h etherdream/driver/libetherdream/etherdream.h
main.o: etherdream/common/protocol.h displaydevice.h transform.h
main.o: ../LIDAR/FrontEndLIDAR/point.h drawing.h ../LIDAR/FrontEndLIDAR/dbg.h
main.o: bezier.h video.h /opt/local/include/cairo/cairo-xlib.h
main.o: /opt/local/include/cairo/cairo.h
main.o: /opt/local/include/cairo/cairo-version.h
main.o: /opt/local/include/cairo/cairo-features.h
main.o: /opt/local/include/cairo/cairo-deprecated.h oschandler.h
main.o: ../LIDAR/FrontEndLIDAR/dest.h
laser.o: etherdream/driver/libetherdream/etherdream.h
laser.o: etherdream/common/protocol.h laser.h displaydevice.h transform.h
laser.o: ../LIDAR/FrontEndLIDAR/point.h drawing.h
laser.o: ../LIDAR/FrontEndLIDAR/dbg.h bezier.h
drawing.o: drawing.h ../LIDAR/FrontEndLIDAR/point.h
drawing.o: etherdream/driver/libetherdream/etherdream.h
drawing.o: etherdream/common/protocol.h ../LIDAR/FrontEndLIDAR/dbg.h bezier.h
drawing.o: transform.h laser.h displaydevice.h
bezier.o: bezier.h ../LIDAR/FrontEndLIDAR/point.h
bezier.o: ../LIDAR/FrontEndLIDAR/dbg.h
video.o: /opt/local/include/cairo/cairo.h
video.o: /opt/local/include/cairo/cairo-version.h
video.o: /opt/local/include/cairo/cairo-features.h
video.o: /opt/local/include/cairo/cairo-deprecated.h
video.o: /opt/local/include/cairo/cairo-xlib.h video.h displaydevice.h
video.o: etherdream/driver/libetherdream/etherdream.h
video.o: etherdream/common/protocol.h transform.h
video.o: ../LIDAR/FrontEndLIDAR/point.h laser.h drawing.h
video.o: ../LIDAR/FrontEndLIDAR/dbg.h bezier.h
transform.o: transform.h ../LIDAR/FrontEndLIDAR/point.h
transform.o: etherdream/driver/libetherdream/etherdream.h
transform.o: etherdream/common/protocol.h drawing.h
transform.o: ../LIDAR/FrontEndLIDAR/dbg.h bezier.h
etherdream/driver/libetherdream/etherdream.o: etherdream/driver/libetherdream/etherdream.h
etherdream/driver/libetherdream/etherdream.o: etherdream/common/protocol.h
