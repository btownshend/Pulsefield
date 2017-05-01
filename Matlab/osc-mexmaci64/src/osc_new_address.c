/**
 *  char[] osc_address_new('ipaddr / hostname', port_num)   or (url)
 *
 */

#include "mex.h"
#include "lo/lo.h"

char maddr[32];
char port[32];
char host[256];

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

  int err;
  int hslen;
  lo_address d;

  if (nrhs != 2 && nrhs!=1) {
    mexErrMsgTxt("Please specify host name/address and port or URL");
    return;
  }
  if (nlhs > 1) {
    mexErrMsgTxt("Too many output arguments.");
    return;
  }

  if(! mxIsChar(prhs[0])) {
    mexErrMsgTxt("Expecting a character array in the first argument (hostname or URL).");
    return;
  }

  if(nrhs==2 && ! mxIsNumeric(prhs[1])) {
    mexErrMsgTxt("Expecting a numeric scalar in the second argument (port number).");
    return;
  }

  hslen = (mxGetM(prhs[0]) * mxGetN(prhs[0])) + 1;
  if(hslen > sizeof(host)+1) {
    mexErrMsgTxt("Maximum url/hostname string length is 255 characters.");
    return;
  }
  err = mxGetString(prhs[0], host, hslen);

  if(err != 0) {
    mexErrMsgTxt("Error reading url/hostname string.");
    return;
  }

  if (nrhs==2) {
      sprintf(port, "%d", (int)(mxGetScalar(prhs[1])));
      d = lo_address_new(host, port);
  } else {
      d = lo_address_new_from_url(host);
  }

  if(! d) {
    mexErrMsgTxt(lo_address_errstr(d));
    return;
  }

  if(sizeof(d) == sizeof(long int)) {
    sprintf(maddr, "osc_address:%lx", (long int)d);
  } else if(sizeof(d) == sizeof(long long int)) {
    sprintf(maddr, "osc_address:%llx", (long long int)d);
  } else {
    mexErrMsgTxt("Unsupported pointer size.");
    return;
  }
  
  plhs[0] = mxCreateString(maddr);

}
