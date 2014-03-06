#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "frontend.h"

static int nsick=1;

void usage(int argc,char *argv[]) {
    fprintf(stderr, "Usage: %s [-r recordfile | -p playfile] (had %d args)\n",argv[0],argc-1);
    exit(1);
}

int main(int argc, char *argv[])
{
    const char *recordFile=NULL;
    const char *playFile=NULL;
    int ch;

    while ((ch=getopt(argc,argv,"r:p:"))!=-1) {
	switch (ch) {
	case 'r':
	    recordFile=optarg;
	    break;
	case 'p':
	    playFile=optarg;
	    break;
	default:
	    usage(argc,argv);
	}
    }
    argc-=optind;
    argv+=optind;
    
    if (argc>0 || (recordFile && playFile) )
	usage(argc,argv);

    if (playFile) {
	// Create a front end with no sensors so it doesn't access any devices
	FrontEnd fe(0);
	// Now playback file through it
	int rc=fe.playFile(playFile);
	if (rc)
	    exit(1);
	else
	    exit(0);
    }

    FrontEnd fe(nsick);
    printf("FrontEnd::FrontEnd() done\n");

    if (recordFile) {
	int rc=fe.startRecording(recordFile);
	if (rc)
	    exit(1);
    }
    
    fe.run();   // Run until quit command received
    fprintf(stderr,"%s exitting\n", argv[0]);
    if (recordFile)
	fe.stopRecording();

    exit(0);
}
