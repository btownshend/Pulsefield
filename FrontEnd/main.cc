#include <stdlib.h>
#include <stdio.h>
#include "frontend.h"

static int ncamera=5;
static int nled=531;

int main(int argc, char *argv[])
{
    if (argc>1)
	ncamera=atoi(argv[1]);
    if (argc>2)
	nled=atoi(argv[2]);
    if (argc>3) {
	fprintf(stderr, "Usage: %s [ncamera [nled]] (had %d args)\n",argv[0],argc-1);
	exit(1);
    }

    FrontEnd fe(ncamera,nled);
    printf("FrontEnd::FrontEnd() done\n");

    fe.run();   // Run until quit command received

    fprintf(stderr,"%s exitting\n", argv[0]);
    exit(0);
}
