CXX=/opt/local/bin/g++
BASEDIR=../..
UTIL=../LIDAR/FrontEndLIDAR    # For dbg
VPATH=.:$(UTIL)
LIBLO=$(BASEDIR)/OSC/liblo-0.26

OPENCV=-lopencv_imgproc -lopencv_core 

ED=etherdream
EDL=$(ED)/driver/libetherdream
INCLUDES=-I$(ED)/driver/libetherdream -I$(ED)/common -I$(UTIL) -I$(LIBLO)
DEBUG=-g -Wall
LDFLAGS=-L$(LIBLO)
LIBES=-llo $(OPENCV)
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES)
CFLAGS=$(DEBUG) $(OPTIMIZE)
CXXSRCS=oschandler.cc main.cc  dbg.cc dest.cc urlconfig.cc laser.cc point.cc drawing.cc bezier.cc
CSRCS=$(EDL)/etherdream.c
SRCS=$(CXXSRCS) $(CSRCS)

osclaser: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

test: test.o $(CSRCS:.c=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

beziertest: bezier.o beziertest.o point.o dbg.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

clean:
	rm *.o

depend:
	/opt/local/bin/makedepend -Y -- $(CXXFLAGS) -- $(SRCS)

# DO NOT DELETE

oschandler.o: ../LIDAR/FrontEndLIDAR/dbg.h oschandler.h
oschandler.o: ../../OSC/liblo-0.26/lo/lo.h
oschandler.o: ../../OSC/liblo-0.26/lo/lo_endian.h
oschandler.o: ../../OSC/liblo-0.26/lo/lo_types.h
oschandler.o: ../../OSC/liblo-0.26/lo/lo_osc_types.h
oschandler.o: ../../OSC/liblo-0.26/lo/lo_errors.h
oschandler.o: ../../OSC/liblo-0.26/lo/lo_lowlevel.h
oschandler.o: ../../OSC/liblo-0.26/lo/lo_macros.h
oschandler.o: ../LIDAR/FrontEndLIDAR/dest.h drawing.h
oschandler.o: ../LIDAR/FrontEndLIDAR/point.h
oschandler.o: etherdream/driver/libetherdream/etherdream.h
oschandler.o: etherdream/common/protocol.h laser.h
oschandler.o: ../LIDAR/FrontEndLIDAR/urlconfig.h
main.o: Laser.h etherdream/driver/libetherdream/etherdream.h
main.o: etherdream/common/protocol.h ../LIDAR/FrontEndLIDAR/point.h
main.o: oschandler.h ../../OSC/liblo-0.26/lo/lo.h
main.o: ../../OSC/liblo-0.26/lo/lo_endian.h
main.o: ../../OSC/liblo-0.26/lo/lo_types.h
main.o: ../../OSC/liblo-0.26/lo/lo_osc_types.h
main.o: ../../OSC/liblo-0.26/lo/lo_errors.h
main.o: ../../OSC/liblo-0.26/lo/lo_lowlevel.h
main.o: ../../OSC/liblo-0.26/lo/lo_macros.h ../LIDAR/FrontEndLIDAR/dest.h
main.o: drawing.h laser.h ../LIDAR/FrontEndLIDAR/dbg.h
laser.o: etherdream/driver/libetherdream/etherdream.h
laser.o: etherdream/common/protocol.h laser.h ../LIDAR/FrontEndLIDAR/point.h
laser.o: ../LIDAR/FrontEndLIDAR/dbg.h
drawing.o: drawing.h ../LIDAR/FrontEndLIDAR/point.h
drawing.o: etherdream/driver/libetherdream/etherdream.h
drawing.o: etherdream/common/protocol.h laser.h ../LIDAR/FrontEndLIDAR/dbg.h
etherdream/driver/libetherdream/etherdream.o: etherdream/driver/libetherdream/etherdream.h
etherdream/driver/libetherdream/etherdream.o: etherdream/common/protocol.h
