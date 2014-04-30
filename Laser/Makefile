BASEDIR=../..
UTIL=../LIDAR/FrontEndLIDAR    # For dbg
VPATH=.:$(UTIL)
LIBLO=$(BASEDIR)/OSC/liblo-0.26

ED=etherdream
EDL=$(ED)/driver/libetherdream
INCLUDES=-I$(ED)/driver/libetherdream -I$(ED)/common -I$(UTIL) -I$(LIBLO)
DEBUG=-g -Wall
LDFLAGS=-L$(LIBLO)
LIBES=-llo
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(INCLUDES)
CFLAGS=$(DEBUG) $(OPTIMZIE)
CXXSRCS=oschandler.cc main.cc  dbg.cc dest.cc urlconfig.cc laser.cc point.cc
CSRCS=$(EDL)/etherdream.c
SRCS=$(CXXSRCS) $(CSRCS)

osclaser: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

test: test.o $(CSRCS:.c=.o)
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
oschandler.o: ../LIDAR/FrontEndLIDAR/dest.h
oschandler.o: ../LIDAR/FrontEndLIDAR/urlconfig.h
main.o: Laser.h oschandler.h ../../OSC/liblo-0.26/lo/lo.h
main.o: ../../OSC/liblo-0.26/lo/lo_endian.h
main.o: ../../OSC/liblo-0.26/lo/lo_types.h
main.o: ../../OSC/liblo-0.26/lo/lo_osc_types.h
main.o: ../../OSC/liblo-0.26/lo/lo_errors.h
main.o: ../../OSC/liblo-0.26/lo/lo_lowlevel.h
main.o: ../../OSC/liblo-0.26/lo/lo_macros.h ../LIDAR/FrontEndLIDAR/dest.h
laser.o: etherdream/driver/libetherdream/etherdream.h
laser.o: etherdream/common/protocol.h laser.h
etherdream/driver/libetherdream/etherdream.o: etherdream/driver/libetherdream/etherdream.h
etherdream/driver/libetherdream/etherdream.o: etherdream/common/protocol.h
