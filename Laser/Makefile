ED=etherdream
EDL=$(ED)/driver/libetherdream
INCLUDES=-I$(ED)/driver/libetherdream -I$(ED)/common
CFLAGS=$(INCLUDES)
CXXFLAGS=$(CFLAGS)
CXXSRCS=
CSRCS=test.c $(EDL)/etherdream.c
SRCS=$(CXXSRCS) $(CSRCS) 

test: $(CSRCS:.c=.o) $(CXXSRCS:.cpp=.o) $(SICKLIB)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBES)
