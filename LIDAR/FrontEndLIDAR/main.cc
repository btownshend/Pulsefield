#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "frontend.h"
#include "dbg.h"
#include "parameters.h"

static int nsick=1;
unsigned int MAXRANGE=12000;

void usage(int argc,char *argv[]) {
    fprintf(stderr, "Usage: %s [-D maxrange] [-R | -r recordfile | -p playfile [-s] [-l] [-x slowfactor] [-m matframes [-M matfile ]] ] [-V]\n",argv[0]);
    fprintf(stderr,"\t-D maxrange\t\tset maximum range in meters\n");
    fprintf(stderr,"\t-R\t\trecord into default filename based on current date and time\n");
    fprintf(stderr,"\t-r file\t\trecord into given file\n");
    fprintf(stderr,"\t-p file\t\tplayback from given file\n");
    fprintf(stderr,"\t\t-s\tsingle-step playback\n");
    fprintf(stderr,"\t\t-l\tloop file continuously\n");
    fprintf(stderr,"\t\t-x x\tslow down playback by a factor of k use -x 0 to run at max speed\n");
    fprintf(stderr,"\t-m matframes\tsave given number of frames to frontend_dump.mat file and exit (or 0 to do all)\n");
    fprintf(stderr,"\t\t-M file\tspecify mat-file name (without suffix)\n");
    fprintf(stderr,"\t-V\t\tenable /vis messages\n");
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
    std::string matfile;
    int matframes=-1;
    bool visoutput=false;
    int argcorig=argc;
    const char **argvorig=(const char **)argv;

    while ((ch=getopt(argc,argv,"d:D:sr:Rp:lx:m:M:V"))!=-1) {
	switch (ch) {
	case 'd':
	    SetDebug(optarg);
	    break;
	case 'D':
	    MAXRANGE=(int)(atof(optarg)*1000);
	    break;
	case 's':
	    singlestep=true;
	    break;
	case 'l':
	    loop=true;
	    break;
	case 'r':
	    recordFile=optarg;
	    break;
	case 'm':
	    matframes=atoi(optarg);
	    break;
	case 'M':
	    matfile=optarg;
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
	case 'V':
	    visoutput=true;
	    break;
	default:
	    usage(argc,argv);
	}
    }
    argc-=optind;
    argv+=optind;
     
    if (argc>0 || (recordFile && playFile) || (matframes>0 && !playFile))
	usage(argc,argv);

    if (playFile) {
	// Create a front end with no sensors so it doesn't access any devices
	FrontEnd fe(0,argcorig,argvorig);
	if (matframes >= 0){
	    if (matfile.empty()) {
		// Use playback file as filename
		matfile=playFile;
		// Remove suffix
		unsigned int dot = matfile.find_last_of(".");
		if (dot!=std::string::npos)
		    matfile.resize(dot);
	    }
	    fe.matsave(matfile,matframes);
	}
	if (visoutput) 
	    fe.getStat(FrontEnd::RANGE,0);
	// Now playback file through it
	do {
	    int rc=fe.playFile(playFile,singlestep,speedFactor);
	    if (rc)
		exit(1);
	} while (loop);   // Keep repeating if loop is set
	exit(0);
    }

    FrontEnd fe(nsick,argcorig,argvorig);
    printf("FrontEnd::FrontEnd() done\n");
    if (visoutput) 
	fe.getStat(FrontEnd::RANGE,0);

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
