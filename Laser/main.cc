#include "lasers.h"
#include "video.h"
#include "oschandler.h"
#include "dbg.h"

void usage(int argc,char *argv[]) {
    fprintf(stderr, "Usage: %s [-n nlaser] [-d debug]\n",argv[0]);
    fprintf(stderr,"\t-d debug\t\tset debug option (e.g -d4, -dLaser:4)\n");
    fprintf(stderr,"\t-n nlaser\t\tnumber of laser devices\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int ch;
    int nlaser=1;
    SetDebug("THREAD:1");   // Print thread names in debug messages, if any

    while ((ch=getopt(argc,argv,"d:n:"))!=-1) {
	switch (ch) {
	case 'n':
	    nlaser=atoi(optarg);
	    break;
	case 'd':
	    SetDebug(optarg);
	    break;
	default:
	    usage(argc,argv);
	}
    }
    argc-=optind;
    argv+=optind;
     
    if (argc>0)
	usage(argc,argv);

    
    dbg("main",1) << "Creating lasers" << std::endl;
    std::shared_ptr<Lasers> lasers(new Lasers(nlaser));
    dbg("main",1) << "Creating video" << std::endl;
    std::shared_ptr<Video> video(new Video(lasers));
    dbg("main",1) << "Opening video" << std::endl;
    video->open();
    dbg("main",1) << "Creating OSCHandler" << std::endl;
    OSCHandler osc(lasers,video);

    dbg("main",1) << "Wait forever..." << std::endl;
    osc.wait();
    dbg("main",1) << "Returned from wait forever." << std::endl;
}
