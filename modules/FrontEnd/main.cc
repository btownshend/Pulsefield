#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "frontend.h"
#include "dbg.h"
#include "parameters.h"
#include "sickio.h"
#include "leg.h"
#include "world.h"

static int nsick=1;
unsigned int MAXRANGE=12000;

void usage(int argc,char *argv[]) {
    fprintf(stderr, "Usage: %s [-n units] [-b] [-B maxrange] [-R | -r recordfile | -p playfile [-L] [-s] [-l] [-x slowfactor] [-F frame1:frameN | -F nframes]  [-m matframes [-M matfile ]] [-P] ] [-v] [-V] [[-D debugfile] -d debug]\n",argv[0]);
    fprintf(stderr,"\t-b\t\tdo background identification of fixed targets in active area\n");
    fprintf(stderr,"\t-B maxrange\tset maximum range in meters\n");
    fprintf(stderr,"\t-R dir\t\trecord into default filename based on current date and time\n");
    fprintf(stderr,"\t-r file\t\trecord into given file\n");
    fprintf(stderr,"\t-p file\t\tplayback from given file\n");
    fprintf(stderr,"\t\t-L\toverlay live as well during playback\n");
    fprintf(stderr,"\t\t-s\tsingle-step playback\n");
    fprintf(stderr,"\t\t-l\tloop file continuously\n");
    fprintf(stderr,"\t\t-x x\tslow down playback by a factor of k use -x 0 to run at max speed\n");
    fprintf(stderr,"\t-F\t\tplayback only given number of frames or frame number range\n");
    fprintf(stderr,"\t-P\tappend performance summary to performance.csv\n");
    fprintf(stderr,"\t-m matframes\tsave given number of frames to frontend_dump.mat file and exit (or 0 to do all)\n");
    fprintf(stderr,"\t\t-M file\tspecify mat-file name (without suffix)\n");
    fprintf(stderr,"\t\t-D file\tspecify debug file name\n");
    fprintf(stderr,"\t-d debug\t\tset debug option (e.g -d4, -dFrontEnd:4)\n");
    fprintf(stderr,"\t-c comment\tlog a comment\n");
    fprintf(stderr,"\t-v\t\tdraw LIDAR ranges (use twice to also draw bg)\n");
    fprintf(stderr,"\t-V\t\tenable /vis messages\n");
    fprintf(stderr,"\t-n nlidar\tnumber of LIDAR units\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    char *recordFile=NULL;
    const char *playFile=NULL;
    int ch;
    bool overlayLive=false;
    bool loop=false;
    bool singlestep=false;
    float speedFactor =1.0f;
    std::string matfile;
    int matframes=-1;
    bool visoutput=false;
    float maxRange=5000;
    int frame1=-1;
    int frameN=-1;
    std::string comments;
    bool savePerfData=false;
    int verbose=0;
    
    SetDebug("THREAD:1");   // Print thread names in debug messages, if any

    while ((ch=getopt(argc,argv,"d:D:bB:sr:R:p:Llx:m:M:vVF:c:Pn:"))!=-1) {
	switch (ch) {
	case 'd':
	    SetDebug(optarg);
	    break;
	case 'b':
	    Background::setRangeOnly(false);
	    break;
	case 'B':
	    maxRange=atof(optarg)*UNITSPERM;
	    break;
	case 'c':
	    // Comment
	    comments+=optarg;
	    comments+="; ";
	    break;
	case 'D':
	    SetDebug("xxx:1",optarg);
	    break;
	case 'F':
	    {
		size_t colonpos=std::string(optarg).find(":");
		if (colonpos==std::string::npos) {
		    frameN=atoi(optarg);
		} else {
		    frame1=atoi(optarg);
		    frameN=atoi(&optarg[colonpos+1]);
		    std::cout << "Running frames " << frame1 << ":" << frameN << std::endl;
		}
	    }
	    break;
	case 'n':
	    nsick=atoi(optarg);
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
	case 'L':
	    overlayLive=true;
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
	    strcpy(recordFile,optarg);
	    strftime(&recordFile[strlen(optarg)],1000-strlen(optarg),"/%Y%m%dT%H%M%S.ferec",localtime(&t));
	    break;
	case 'p':
	    playFile=optarg;
	    break;
	case 'P':
	    savePerfData=true;
	    break;
	case 'x':
	    speedFactor=atof(optarg);
	    break;
	case 'v':
	    verbose++;
	    World::setDrawRange(true);
	    if (verbose>1)
		World::setDrawBG(true);
	    break;
	case 'V':
	    visoutput=true;
	    break;
	default:
	    usage(argc,argv);
	}
    }
    if (argc>optind || (matframes>0 && !playFile))
	usage(argc,argv);

    dbg("main",1) << "Comment: " << comments << std::endl;

    Leg::setup(50.0,nsick);   // FIXME: Assumes 50Hz scan freq

    if (playFile) {
	// Create a front end with no sensors so it doesn't access any devices
	FrontEnd fe(overlayLive?nsick:0,maxRange,argc,(const char **)argv);
	if (matframes >= 0){
	    if (matfile.empty()) {
		// Use playback file as filename
		matfile=playFile;
		// Remove suffix
		int dot = matfile.find_last_of(".");
		if (dot!=std::string::npos)
		    matfile.resize(dot);
	    }
	    fe.matsave(matfile,matframes);
	}
	if (visoutput) 
	    fe.getStat(FrontEnd::RANGE,0);
	if (recordFile) {
	    int rc=SickIO::startRecording(recordFile,comments);
	    if (rc)
		exit(1);
	}
	// Now playback file through it
	do {
	    int rc=fe.playFile(playFile,singlestep,speedFactor,overlayLive,frame1,frameN,savePerfData,nsick);
	    savePerfData=false;   // Only save it once if looping
	    if (rc)
		exit(1);
	} while (loop);   // Keep repeating if loop is set
	exit(0);
    }

    FrontEnd fe(nsick,maxRange,argc,(const char **)argv);
    printf("FrontEnd::FrontEnd() done\n");
    if (visoutput) 
	fe.getStat(FrontEnd::RANGE,0);

    if (recordFile) {
	int rc=SickIO::startRecording(recordFile,comments);
	if (rc)
	    exit(1);
    }
    
    fe.run();   // Run until quit command received
    fprintf(stderr,"%s exitting\n", argv[0]);
    if (recordFile)
	SickIO::stopRecording();

    exit(0);
}
