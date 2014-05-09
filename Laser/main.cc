#include "laser.h"
#include "video.h"
#include "oschandler.h"
#include "dbg.h"

void usage(int argc,char *argv[]) {
    fprintf(stderr, "Usage: %s [-d debug]\n",argv[0]);
    fprintf(stderr,"\t-d debug\t\tset debug option (e.g -d4, -dLaser:4)\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int ch;
    while ((ch=getopt(argc,argv,"d:D:sr:Rp:lx:m:M:V"))!=-1) {
	switch (ch) {
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

    Laser laser;
    laser.open();
    Video video;
    video.open();

    OSCHandler osc(0,&laser,&video);
    osc.wait();
}
