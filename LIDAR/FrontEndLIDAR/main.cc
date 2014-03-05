#include <stdlib.h>
#include <stdio.h>
#include "frontend.h"

static int nsick=1;

int main(int argc, char *argv[])
{
	if (argc>1)
		nsick=atoi(argv[1]);
	if (argc>2) {
		fprintf(stderr, "Usage: %s [nsick] (had %d args)\n",argv[0],argc-1);
		exit(1);
	}

	FrontEnd fe(nsick);
	printf("FrontEnd::FrontEnd() done\n");

	fe.run();   // Run until quit command received
	fprintf(stderr,"%s exitting\n", argv[0]);
	exit(0);
}
