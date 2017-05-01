MATLABROOT=/Applications/Matlab
CC=clang
LIBS=-L/opt/local/lib
CFLAGS=-Wall -m64 -O3 -I$(MATLABROOT)/extern/include -I/opt/local/include
MEXFLAG=-m64 -shared -DMATLAB_MEX_FILE -I$(MATLABROOT)/extern/include -Wl,--export-all-symbols $(LIBS) $(MEXSRC) -o $(MEXTGT).mexw64 -I/opt/local/include

osc_recv: osc_recv.c
	$(CC) $(MEXFLAG) osc_recv.c $(LIBS) -llo
