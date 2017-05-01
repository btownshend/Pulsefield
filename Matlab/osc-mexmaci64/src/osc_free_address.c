/**
 *  void osc_address_free('osc_address:........')
 *
 */

#include "mex.h"
#include "lo/lo.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

  int err;
  lo_address d;
  int mlen;
  char maddr[32];

  if (nrhs != 1) {
    mexErrMsgTxt("Expecting one argument");
    return;
  }
  if (nlhs > 0) {
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

  //  mexPrintf("sizeof(d)=%d, sizeof(void*)=%d, sizeof(int)=%d\n", sizeof(d), sizeof(void*), sizeof(int));

  if(sizeof(d) == sizeof(long int)) {
      err = sscanf(maddr, "osc_address:%lx", (long int *)&d);
  } else {
    mexErrMsgTxt("Unsupported pointer size.");
  }

  if(err < 0 || (! d)) {
    mexErrMsgTxt("Error scanning input string.");
    return;
  }

  // mexPrintf("lo_address_free(%x)\n",d);
  lo_address_free(d);
  
}
