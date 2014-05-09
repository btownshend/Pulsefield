#include "laser.h"
#include "video.h"
#include "oschandler.h"
#include "dbg.h"

void usage(int argc,char *argv[]) {
    fprintf(stderr, "Usage: %s [-n nlaser] [-d debug]\n",argv[0]);
    fprintf(stderr,"\t-d debug\t\tset debug option (e.g -d4, -dLaser:4)\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int ch;
    int nlaser=4;

    while ((ch=getopt(argc,argv,"d:"))!=-1) {
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

    
    Lasers lasers(nlaser);
    Video video(lasers);
    video.open();

    OSCHandler osc(lasers,&video);
    osc.wait();
}
