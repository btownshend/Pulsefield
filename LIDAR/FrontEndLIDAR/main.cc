#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "frontend.h"

static int nsick=1;

void usage(int argc,char *argv[]) {
    fprintf(stderr, "Usage: %s [-R | -r recordfile | -p playfile [-s] [-l] [-x slowfactor] ] (had %d args)\n",argv[0],argc-1);
    exit(1);
}

int main(int argc, char *argv[])
{
    char *recordFile=NULL;
    const char *playFile=NULL;
    int ch;
    bool loop=false;
    bool singlestep=false;
    float speedFactor =1.0f;

    while ((ch=getopt(argc,argv,"sr:Rp:lx:"))!=-1) {
	switch (ch) {
	case 's':
	    singlestep=true;
	    break;
	case 'l':
	    loop=true;
	    break;
	case 'r':
	    recordFile=optarg;
	    break;
	case 'R':
	    recordFile=new char[1000];
	    time_t t;
	    time(&t);
	    strftime(recordFile,1000,"%Y%m%dT%H%M%S.ferec",localtime(&t));
	    break;
	case 'p':
	    playFile=optarg;
	    break;
	case 'x':
	    speedFactor=atof(optarg);
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
	do {
	    int rc=fe.playFile(playFile,singlestep,speedFactor);
	    if (rc)
		exit(1);
	} while (loop);   // Keep repeating if loop is set
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
