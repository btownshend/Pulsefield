#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#include "camio.h"
#include "visible.h"
#include "frontend.h"

const int FrontEnd::serverPort=7770;

int debug=0;

static void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
static int generic_handler(const char *path, const char *types, lo_arg **argv,int argc, lo_message msg, void *user_data) {
    int i;
    printf("Unhandled Message Rcvd: %s (", path);
    for (i=0; i<argc; i++) {
	printf("%c",types[i]);
    }
    printf("): ");
    for (i=0; i<argc; i++) {
	lo_arg_pp((lo_type)types[i], argv[i]);
	printf(", ");
    }
    printf("\n");
    fflush(stdout);
    return 1;
}

static int quit_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data) {
    printf("Quitting\n\n");
    exit(0);
}

/* Handler stubs */
static int start_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->startStop(true); return 0; }
static int stop_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->startStop(false); return 0; }

static int setPos_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setPos(argv[0]->i,argv[1]->i,argv[2]->i,argv[3]->i,argv[4]->i,argv[5]->i); return 0; }
static int setFPS_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setFPS(argv[0]->i); return 0; }
static int setRes_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setRes(argv[0]->i,&argv[1]->s); return 0; }
static int setRefImage_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setRefImage(argv[0]->i,argv[1]->i,argv[2]->i,argv[3]->i, &argv[4]->s); return 0; }
static int setROI_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setROI(argv[0]->i,argv[1]->i,argv[2]->i,argv[3]->i,argv[4]->i); return 0; }
static int setUpdateTC_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setUpdateTC(argv[0]->f); return 0; }
static int setCorrThresh_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->setCorrThresh(argv[0]->f); return 0; }

static int getCorr_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::CORR); return 0; }
static int getRefImage_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::REFIMAGE); return 0; }
static int getImage_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::IMAGE); return 0; }

static int addDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->addDest(&argv[0]->s,argv[1]->i); return 0; }
static int rmDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmDest(&argv[0]->s,argv[1]->i); return 0; }
static int rmAllDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmAllDest(); return 0; }

FrontEnd::FrontEnd(int _ncamera, int _nled) {
    ncamera=_ncamera;
    nled=_nled;
    frame = 0;

    printf("Starting %d cameras...",ncamera);fflush(stdout);
    cameras = new  CamIO*[ncamera];
    vis = new Visible*[ncamera];

    int ids[]={1,2,5,3,4};
    for (int i=0;i<ncamera;i++) {
	cameras[i]=new CamIO(ids[i]);
	vis[i] = new Visible(nled);
    }
    printf("done\n");fflush(stdout);

    /* start a new server on port 7770 */
    char cbuf[10];
    sprintf(cbuf,"%d", serverPort);
    s = lo_server_new(cbuf, error);

    /* add method that will match the path /quit with no args */
    lo_server_add_method(s, "/quit", "", quit_handler, NULL);

    /* add methods */
    lo_server_add_method(s,"/vis/start","",start_handler,this);
    lo_server_add_method(s,"/vis/stop","",stop_handler,this);

    lo_server_add_method(s,"/vis/set/pos","iiiiii",setPos_handler,this);
    lo_server_add_method(s,"/vis/set/fps","i",setFPS_handler,this);
    lo_server_add_method(s,"/vis/set/updatetc","f",setUpdateTC_handler,this);
    lo_server_add_method(s,"/vis/set/corrthresh","f",setCorrThresh_handler,this);
    lo_server_add_method(s,"/vis/set/res","is",setRes_handler,this);
    lo_server_add_method(s,"/vis/set/refimage","iiiis",setRefImage_handler,this);
    lo_server_add_method(s,"/vis/set/roi","iiiii",setROI_handler,this);

    
    lo_server_add_method(s,"/vis/get/corr","",getCorr_handler,this);
    lo_server_add_method(s,"/vis/get/refimage","",getRefImage_handler,this);
    lo_server_add_method(s,"/vis/get/image","",getImage_handler,this);

    lo_server_add_method(s,"/vis/dest/add","si",addDest_handler,this);
    lo_server_add_method(s,"/vis/dest/remove","si",rmDest_handler,this);
    lo_server_add_method(s,"/vis/dest/clear","",rmAllDest_handler,this);

    /* add method that will match any path and args if they haven't been caught above */
    lo_server_add_method(s, NULL, NULL, generic_handler, NULL);
}

void FrontEnd::run() {
    int retval;

    int lo_fd = lo_server_get_socket_fd(s);
    while (true) {  /* Forever */
	fd_set rfds;

	/* Setup file descriptor to watch on */
	FD_ZERO(&rfds);

	/* get the file descriptor of the server socket and watch for messages */
	FD_SET(lo_fd, &rfds);
	int maxfd=lo_fd;

	/* Watch for data from the cameras */
	for (int i=0;i<ncamera;i++)  {
	    int sock=cameras[i]->getSocket();
	    if (sock>=0) {
		FD_SET(sock,&rfds);
		if (sock>maxfd)
		    maxfd=sock;
	    }
	}

	retval = select(maxfd + 1, &rfds, NULL, NULL, NULL); /* no timeout */
	//	printf("rfds=0x%lx\n", *(unsigned long *)&rfds);
	if (retval == -1) {
	    printf("select() error\n");
	    exit(1);
	} else if (retval > 0) {
	    // Process OSC messages
	    if (FD_ISSET(lo_fd, &rfds))
		lo_server_recv_noblock(s, 0);

	    // Read input from cameras
	    int haveAllFrames=1;
	    for (int i=0;i<ncamera;i++) {
		if (cameras[i]->isRunning() && FD_ISSET(cameras[i]->getSocket(),&rfds)) {
		    // printf("Reading from  camera %d\n", i);
		    if (cameras[i]->read() < 0)
			exit(1);
		}
		haveAllFrames=haveAllFrames && cameras[i]->getFrame()->isValid();
	    }

	    // Process if all the cameras have new frames
	    if (haveAllFrames) {
		processFrames();
	    }
	}
    }
}


void FrontEnd::processFrames() {
    const bool dumpVis=false;

    if (debug)
	printf("Processing frame %d\n",frame);
    double minus=1e99, maxus=0;
    for (int i=0;i<ncamera;i++) {
	Frame *frame=cameras[i]->getFrame();
	double us=frame->getTimestamp().tv_usec/1e6 + frame->getTimestamp().tv_sec;
	if (us<minus)
	    minus=us;
	else if (us>maxus)
	    maxus=us;
	
	if (debug)
	    printf("Camera %d: image size is %d x %d (%s):  ", i, frame->getWidth(), frame->getHeight(), frame->isColor()?"color":"grayscale");
	if (frame->getWidth() != vis[i]->getRefWidth() || frame->getHeight() != vis[i]->getRefHeight()) {
	    printf("Setting targets for camera %d to current frame\n", i);
	    //vis[i]->setRefImage(frame->getWidth(),frame->getHeight(),frame->isColor()?3:1,frame->getImage());
	    vis[i]->setRefImage(frame->getWidth(),frame->getHeight(),frame->isColor()?3:1,(float *)0);
	} else {
	    struct timeval tstart, tend;
	    if (debug)
		gettimeofday(&tstart,NULL);
	    vis[i]->processImage(frame,cameras[i]->getFPS());

	    if (debug) {
		gettimeofday(&tend,NULL);
		printf("processImage() took %.3f msec.\n",(tend.tv_usec-tstart.tv_usec)/1000.0+(tend.tv_sec-tstart.tv_sec));
	    }
	    tosend |= VIS|CORR;
	    if (dumpVis) {
		printf("V[%d]=",i);
		for (int j=0;j<nled;j++)
		    printf("%d",vis[i]->getVisible()[j]);
		printf("\n");
	    }
	}
    }
    if (debug)
	printf("Frame jitter = %.1f msec.\n", (maxus-minus)*1000);

    for (int i=0;i<ncamera;i++)
	cameras[i]->getFrame()->clearValid();   // Ready to rewrite

    sendMessages();
    frame=frame+1;
}

// Send all messages that are in the tosend list to the destinations
void FrontEnd::sendMessages() {
    if (debug>1)
	printf("sendMessages()  tosend=%ld, ndest=%d\n",tosend,dests.count());

    for (int i=0;i<dests.count();i++) {
	char cbuf[10];
	sprintf(cbuf,"%d",dests.getPort(i));
	lo_address addr = lo_address_new(dests.getHost(i), cbuf);
	lo_send(addr,"/vis/beginframe","i",frame);
	for (int c=0;c<ncamera;c++) {
	    struct timeval ts=vis[c]->getTimestamp();
	    if (tosend & VIS) {
		// Send visibility info
		lo_blob data = lo_blob_new(nled,vis[c]->getVisible());
		byte *data_ptr=(byte *)lo_blob_dataptr(data);
		int n0=0,n1=0,n2=0;
		for (int j=0;j<nled;j++)
		    if (data_ptr[j]==0)
			n0++;
		    else if (data_ptr[j]==1)
			n1++;
		    else if (data_ptr[j]==2)
			n2++;
		if (debug>1)
		    printf("Sending VIS%d (%d/%d/%d) to %s:%d\n", c, n0, n1, n2, dests.getHost(i),dests.getPort(i));
		lo_send(addr, "/vis/visible","iiiifb",c,frame,ts.tv_sec, ts.tv_usec, vis[c]->getCorrThresh(), data);
		lo_blob_free(data);
	    }
	    if (tosend & CORR) {
		// Send correlations
		if (debug>1)
		    printf("Sending CORR to %s:%d\n", dests.getHost(i),dests.getPort(i));
		lo_blob data = lo_blob_new(nled*sizeof(float),vis[c]->getCorr());
		lo_send(addr, "/vis/corr","iiiib",c,frame,ts.tv_sec, ts.tv_usec, data);
		lo_blob_free(data);
	    }
	    if (tosend & REFIMAGE) {
		if (debug)
		    printf("Sending REFIMAGE to %s:%d\n", dests.getHost(i),dests.getPort(i));
		const char *filename=vis[c]->saveRef();    // Save frame in file, retrieve filename (as float)
		printf("Saved image in %s\n", filename);
		int width=vis[c]->getRefWidth();
		int height=vis[c]->getRefHeight();
		int depth=vis[c]->getRefDepth();
		lo_send(addr, "/vis/refimage","iiiiiiiss",c,frame,ts.tv_sec,ts.tv_usec, width, height,depth,"f",filename);
	    }	   
	    if (tosend & IMAGE) {
		if (debug)
		    printf("Sending IMAGE to %s:%d\n", dests.getHost(i),dests.getPort(i));
		Frame *fptr=cameras[c]->getFrame();
		const char *filename=fptr->saveImage();    // Save frame in file, retrieve filename (as bytes)
		printf("Saved image in %s\n", filename);
		int width=fptr->getWidth();
		int height=fptr->getHeight();
		int depth=fptr->isColor()?3:1;
		lo_send(addr, "/vis/image","iiiiiiiss",c,frame,ts.tv_sec,ts.tv_usec, width, height,depth,"b",filename);
	    }
	}
	lo_send(addr,"/vis/endframe","i",frame);
	lo_address_free(addr);
    }
    tosend=0;
}

void FrontEnd::startStop(bool start) {
    printf("FrontEnd: %s\n", start?"start":"stop");
    for (int i=0;i<ncamera;i++)
	cameras[i]->startStop(start);
}

void FrontEnd::setPos(int camera, int led, int xpos, int ypos, int twidth, int theight) {
    if (led==0)
	printf("setPos(%d,%d,%d,%d,%d,%d)\n",camera,led,xpos,ypos,twidth,theight);

    if (camera<0 || camera>=ncamera) {
	fprintf(stderr,"setPos: bad camera number: %d\n", camera);
	return;
    }
    if (led<0 || led>=nled) {
	fprintf(stderr,"setPos: bad led number: %d\n", led);
	return;
    }
    if (0) {
	// NO - now they are in received image coordinates
	/* The received xpos,ypos are for the full image space of the camera, but the images we're receiving are only the ROI -- need to offset the positions */
	if (xpos!=-1)
	    xpos-=cameras[camera]->getX0();
	if (ypos!=-1)
	    ypos-=cameras[camera]->getY0();
    }

    vis[camera]->setPosition(led,xpos,ypos,twidth,theight);
}

void FrontEnd::setFPS(int fps) {
    for (int i=0;i<ncamera;i++)
	cameras[i]->setFPS(fps);
}

// Change time constant (in seconds) for updating reference images
// Use 0 to not update
void FrontEnd::setUpdateTC(float updateTime) {
    Visible::setUpdateTimeConstant(updateTime);
}

void FrontEnd::setCorrThresh(float thresh) {
    Visible::setCorrThresh(thresh);
}

void FrontEnd::setRes(int camera, const char *res) {
    printf("setRes(%d,%s)\n", camera, res);
    cameras[camera]->setRes(res);
}

void FrontEnd::setROI(int camera, int x0, int y0, int x1, int y1) {
    if (camera<0 || camera>=ncamera) {
	fprintf(stderr,"setROI: bad camera number: %d\n", camera);
	return;
    }
    cameras[camera]->setROI(x0,y0,x1,y1);
}

void FrontEnd::setRefImage(int camera, int imgwidth, int imgheight, int imgdepth, const char *filename) {
    printf("setRefImage(%d,%d,%d,%d,%s)\n", camera, imgwidth, imgheight, imgdepth, filename);
    if (camera<0 || camera>=ncamera) {
	fprintf(stderr,"setRefImage: bad camera number: %d\n", camera);
	return;
    }
    int fd=open(filename,O_RDONLY);
    if (fd<0) {
	perror(filename);
	return;
    }
    int nelem=imgwidth*imgheight*imgdepth;
    float *buffer = new float[nelem];
    int ntotal=0;
    while (ntotal < nelem*sizeof(*buffer)) {
	int nbytes=sizeof(*buffer)*nelem-ntotal;
	int nr=read(fd,&buffer[ntotal],nbytes);
	printf("Read %d/%d bytes from %s\n", nr,nbytes,filename); 
	if (nr<0) {
	    perror(filename);
	    break;
	}
	if (nr==0) {
	    fprintf(stderr,"Premature EOF on %s; read %d/%ld bytes\n", filename, ntotal,nelem*sizeof(float));
	    break;
	}
	ntotal+=nr;
    }
    close(fd);
    vis[camera]->setRefImage(imgwidth, imgheight, imgdepth, buffer);
    delete [] buffer;
}

void FrontEnd::getStat(long int stat) {
    tosend |= stat;
}

void FrontEnd::addDest(const char *host, int port) {
    dests.add(host,port);
}

void FrontEnd::rmDest(const char *host, int port) {
    dests.remove(host,port);
}

void FrontEnd::rmAllDest() {
    dests.removeAll();
}
