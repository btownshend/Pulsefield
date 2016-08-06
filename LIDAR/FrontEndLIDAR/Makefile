BASEDIR=../../..

UTIL=../Util
VPATH=.:$(UTIL)

LIBLOLIBDIR=/opt/local/lib

MATLAB=/Applications/MATLAB
MATLABINC=$(MATLAB)/extern/include
MATLABLIBDIR=$(MATLAB)/bin/maci64
MATLABLIBES=-lmx -lmat

SICKTOOLBOX=$(BASEDIR)/sicktoolbox/trunk/c++/drivers
SICKLIBDIR=$(SICKTOOLBOX)/lms5xx/sicklms5xx/.libs
SICKLIB=$(SICKLIBDIR)/libsicklms5xx.a 

CAIROINC=/opt/local/include/cairo
CAIROLIBDIR=/opt/local/lib
CAIROLIBES=-lcairo -lX11

OPENCVINC=-I/opt/local/include
OPENCVLIBDIR=/opt/local/lib
OPENCVLIBES=-lopencv_ml -lopencv_video -lopencv_core

#CXX=/opt/local/bin/g++
#DEBUG=-g -ggdb -Wall -D_GLIBCXX_DEBUG
CXX=clang++
DEBUG=-g  -Wall
PROFILE=
OPTIMIZE=-Ofast
CXXFLAGS=-pthread $(DEBUG) $(OPTIMIZE) $(PROFILE) -I$(UTIL) -I$(SICKTOOLBOX)/lms5xx -I$(SICKTOOLBOX)/base/src -I$(MATLABINC) -I$(CAIROINC) $(OPENCVINC) -I/opt/X11/include -Wno-overloaded-virtual
CFLAGS=$(DEBUG)  $(OPTIMIZE)
LDFLAGS=-L$(LIBLOLIBDIR) -L$(MATLABLIBDIR) $(DEBUG) $(PROFILE) -L$(CAIROLIBDIR)
LIBES=-llo $(SICKLIB) $(MATLABLIBES) $(CAIROLIBES) -L$(OPENCVLIBDIR) $(OPENCVLIBES) -g 
CXXSRCS=frontend.cc handlers.cc dest.cc main.cc urlconfig.cc sickio.cc background.cc world.cc snapshot.cc vis.cc person.cc dbg.cc point.cc worlddraw.cc lookuptable.cc  leg.cc legstats.cc groups.cc kalmanfilter.cc findtargets.cc configuration.cc
CSRCS=
SRCS=$(CXXSRCS) $(CSRCS)
all: frontend
	echo Make sure $(MATLABLIBDIR) is in DYLD_LIBRARY_PATH 

frontend: $(CSRCS:.c=.o) $(CXXSRCS:.cc=.o) $(SICKLIB)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)

mixtest: mixtest.o gaussianmixture.o dbg.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)  -L$(OPENCVLIBDIR)  $(OPENCVLIBES) 

depend:
	/opt/local/bin/makedepend -Y -- $(CXXFLAGS) -- $(SRCS)

clean:
	rm *.o

# DO NOT DELETE
