/**
 *  void osc_server_recv('osc_server:........', timeout)
 */

#include <string.h>
#include "mex.h"
#include "matrix.h"
#include "lo/lo.h"

char maddr[256];
char tph[1024];

#define MAXMSGS 512

const char* fieldnames[] = {"path", "tt", "data","src"};

mxArray* rm[MAXMSGS];  // for parts of bundled messages recv'd
int rmi;           // current number of messages on the stack 
int rmi_start;     // index of the first message (might not be zero)

int parse_osc_message(
		      const char *path,
		      const char *types,
		      lo_arg **argv,
		      int argc,
		      lo_message msg,
		      void *user_data
		      )
{
    int i;
    int n;
    lo_arg t;
    mxArray* p;
    mxArray* q;
    mxArray* s;

    p = mxCreateStructMatrix(1,1, sizeof(fieldnames)/sizeof(fieldnames[0]), fieldnames);
    if(rmi > MAXMSGS) {
	// data at current position can be destroyed
	mxDestroyArray(rm[rmi % MAXMSGS]);

	// starting position moves (its a ring buffer containing the N most recent packets)
	rmi_start = (rmi_start + 1) % MAXMSGS;
    }
    rm[rmi % MAXMSGS] = p;
    rmi++;

    mxSetField(p, 0, "path", mxCreateString(path));
    sprintf(tph, "%s", types);

    q = mxCreateCellMatrix(1, argc);

    for (i = 0; i < argc; i++) {
	if(lo_is_numerical_type(types[i])) {
	    lo_coerce(LO_DOUBLE, &t, types[i], argv[i]);
	    mxSetCell(q, i, mxCreateDoubleScalar(t.d));
	}
	else if(lo_is_string_type(types[i])) {
	    s = mxCreateString(&(argv[i]->s));
	    if(s == NULL) {
		mexErrMsgTxt("Error creating string");
	    } else {
		mxSetCell(q, i, s);
	    }
	}
	else if(types[i] == LO_TRUE) {
	    tph[i] = 'L';
	    mxSetCell(q, i, mxCreateLogicalScalar(1));
	}
	else if(types[i] == LO_FALSE) {
	    tph[i] = 'L';
	    mxSetCell(q, i, mxCreateLogicalScalar(0));
	}
	else if(types[i] == LO_NIL) {
	    tph[i] = 'd';
	    mxSetCell(q, i, mxCreateDoubleScalar(mxGetNaN()));
	}
	else if(types[i] == LO_INFINITUM) {
	    tph[i] = 'd';
	    mxSetCell(q, i, mxCreateDoubleScalar(mxGetInf()));
	} else if(types[i] == LO_BLOB) {
	    tph[i] = 'b';
	    lo_blob blob = (lo_blob)argv[i];
	    int blobSize=lo_blob_datasize(blob);
	    // mexPrintf("Got BLOB of size %d\n", blobSize);
	    unsigned char *blobData =lo_blob_dataptr(blob);
	    mxArray *array = mxCreateNumericMatrix(1,blobSize,mxUINT8_CLASS,0);
	    unsigned char *data = (unsigned char *)mxGetData(array);
	    if (data==0)
		mexErrMsgTxt("Unable to get data pointer for setting BLOB data\n");
	    memmove(data,blobData,blobSize);
	    mxSetCell(q, i,array);
	    // lo_blob_free(blob);
	} else {
	    mexPrintf("Unrecognized type: %c\n", types[i]);
	    mexErrMsgTxt("Sorry I don't know how to handle args of unknown type.");
	}
    }

    mxSetField(p, 0, "tt", mxCreateString(tph));
    mxSetField(p, 0, "data", q);
    //mexPrintf("srcurl=%s\n",lo_address_get_url(lo_message_get_source(msg)));
    //mexEvalString("drawnow;");
    mxSetField(p, 0, "src", mxCreateString(lo_address_get_url(lo_message_get_source(msg))));
    return 1;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

    int err;
    lo_server s;
    int mlen;
    int timeout;
    mxArray* p;
    mxArray* q;
    int i;
  
    if (nrhs > 2) {
	mexErrMsgTxt("Expecting at most two arguments");
	return;
    }
    if (nlhs > 1) {
	mexErrMsgTxt("Too many output arguments.");
	return;
    }

    if(! mxIsChar(prhs[0])) {
	mexErrMsgTxt("Expecting a character array in the first argument.");
	return;
    }

    mlen = (mxGetM(prhs[0]) * mxGetN(prhs[0])) + 1;
    if(mlen > 32) {
	mexErrMsgTxt("Input string is too long.");
	return;
    }
    err = mxGetString(prhs[0], maddr, mlen);

    if(err != 0) {
	mexErrMsgTxt("Error reading input string.");
	return;
    }

    if(sizeof(s) == sizeof(long int)) {
	err = sscanf(maddr, "osc_server:%lx", (long int *)&s);
    } else if(sizeof(s) == sizeof(long long int)) {
	err = sscanf(maddr, "osc_server:%llx", (long long int*)&s);
    } else {
	mexErrMsgTxt("Unsupported pointer size.");
	return;
    }

    if(err < 0 || (! s)) {
	mexErrMsgTxt("Error scanning input string.");
	return;
    }

    if(nrhs == 2) {
	timeout = (int)(1000.0 * mxGetScalar(prhs[1]));
    } else {
	timeout = -1;
    }

    rmi = 0;
    rmi_start = 0;
  
    lo_server_add_method(s, NULL, NULL, parse_osc_message, p);
  
    // take packets from the server without blocking until the buffer is empty...
    // limit to 1000 packets in case the stream is coming in faster than we can process...
    i = 0;
    while(rmi < MAXMSGS * 100) { // try to put some sane limit on how much we will pull...
	i = rmi;
	lo_server_recv_noblock(s, 0);
	if(i == rmi) {
	    break;
	}
    }
  
    // if we didn't read anything in that attempt then wait for n milliseconds for a packet to come in...
    if(i == 0 && timeout >= 0) {
	int t1=timeout/2;
	//mexPrintf("i=0, timeout=%d\n", t1);
	// For some reason, this sometime returns immediately even if no msgs and timeout>0
	// Probably reading partial bytes of a msg, causing this to return, but haven't yet triggered callback
	// But, doing it twice seems to fix it.
	int nbytes=lo_server_recv_noblock(s, t1);
	//mexPrintf("rmi=%d, nbytes=%d\n", rmi,nbytes);
	if (rmi==0) {
	    int t2=timeout-t1;
	    nbytes=lo_server_recv_noblock(s, t2);
	    //mexPrintf("rmi=%d, nbytes=%d\n", rmi,nbytes);
	}
    } else if(i == 0) { // timeout is -1, i.e. no timeout
	// note there is no way to abort this from matlab...
	lo_server_recv(s);
    }
  
    lo_server_del_method(s, NULL, NULL);

    if(rmi == 0) {
	plhs[0] = mxCreateCellMatrix(0,0); // empty cell array
    } else {
	q = mxCreateCellMatrix(1, rmi > MAXMSGS ? MAXMSGS : rmi);

	// read the pointers out of the ring buffer...
	for(i = 0; i < MAXMSGS && i < rmi; i++) {
	    mxSetCell(q, i, rm[(rmi_start + i) % MAXMSGS]);
	}

	plhs[0] = q;
    }

}

