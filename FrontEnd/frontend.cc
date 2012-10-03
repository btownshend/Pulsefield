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
#include "urlconfig.h"

int debug=0;

static void error(int num, const char *msg, const char *path)
{
    fprintf(stderr,"liblo server error %d in path %s: %s\n", num, path, msg);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
static int generic_handler(const char *path, const char *types, lo_arg **argv,int argc, lo_message , void *) {
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

static bool doQuit = false;

static int quit_handler(const char *, const char *, lo_arg **, int, lo_message , void *) {
    printf("Received /quit command, quitting\n");
    doQuit = true;
    return 0;
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

static int getCorr_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::CORR,argv[0]->i); return 0; }
static int getRefImage_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::REFIMAGE,argv[0]->i); return 0; }
static int getImage_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->getStat(FrontEnd::IMAGE,argv[0]->i); return 0; }

static int addDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->addDest(&argv[0]->s,argv[1]->i); return 0; }
static int addDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->addDest(msg,argv[0]->i); return 0; }
static int rmDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmDest(&argv[0]->s,argv[1]->i); return 0; }
static int rmDestPort_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmDest(msg,argv[0]->i); return 0; }
static int rmAllDest_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->rmAllDest(); return 0; }
static int ping_handler(const char *path, const char *types, lo_arg **argv, int argc,lo_message msg, void *user_data) {    ((FrontEnd *)user_data)->ping(msg,argv[0]->i); return 0; }

FrontEnd::FrontEnd(int _ncamera, int _nled) {
    ncamera=_ncamera;
    nled=_nled;
    if (debug)
	printf("FrontEnd::FrontEnd(%d,%d)\n", ncamera,nled);

    frame = 0;

    cameras = new  CamIO*[ncamera];
    vis = new Visible*[ncamera];


    URLConfig urls("/Users/bst/DropBox/PeopleSensor/config/urlconfig.txt");

    serverPort=urls.getPort("FE");
    if (serverPort<0) {
	fprintf(stderr,"Invalid server port retrieved from %s when looking for FE\n", urls.getFilename());
	exit(1);
    }


    /* start a new server on OSC port  */
    char cbuf[10];
    sprintf(cbuf,"%d", serverPort);
    s = lo_server_new(cbuf, error);
    if (s==0) {
	fprintf(stderr,"Unable to start server on port %d -- perhaps another instance is running\n", serverPort);
	exit(1);
    }
    printf("Started server on port %d\n", serverPort);


    /* Start cameras */
    printf("Initializing with %d cameras...",ncamera);fflush(stdout);
    for (int i=0;i<ncamera;i++) {
	char ident[20];
	sprintf(ident,"CA%d",i+1);
	int port=urls.getPort(ident);
	if (port<0) {
	    fprintf(stderr,"Unable to locate %s in config file %s\n", ident, urls.getFilename());
	    exit(1);
	}
	const char *host=urls.getHost(ident);
	cameras[i]=new CamIO(i+1,host,port);
	vis[i] = new Visible(nled);
    }
    printf("done\n");fflush(stdout);

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
    lo_server_add_method(s,"/ping","i",ping_handler,this);
    
    lo_server_add_method(s,"/vis/get/corr","i",getCorr_handler,this);
    lo_server_add_method(s,"/vis/get/refimage","i",getRefImage_handler,this);
    lo_server_add_method(s,"/vis/get/image","i",getImage_handler,this);

    lo_server_add_method(s,"/vis/dest/add","si",addDest_handler,this);
    lo_server_add_method(s,"/vis/dest/add/port","i",addDestPort_handler,this);
    lo_server_add_method(s,"/vis/dest/remove","si",rmDest_handler,this);
    lo_server_add_method(s,"/vis/dest/remove/port","i",rmDestPort_handler,this);
    lo_server_add_method(s,"/vis/dest/clear","",rmAllDest_handler,this);

    /* add method that will match any path and args if they haven't been caught above */
    lo_server_add_method(s, NULL, NULL, generic_handler, NULL);
    
    /* add default destinations */
    // addDest("localhost",7771);

    /* Set to always send only VIS information */
    sendAlways=VIS;
}

FrontEnd::~FrontEnd() {
    for (int i=0;i<ncamera;i++) {
	delete cameras[i];
	delete vis[i];
    }
    delete [] cameras;
    delete [] vis;
    lo_server_free(s);
}

void FrontEnd::run() {
    int retval;

    int lo_fd = lo_server_get_socket_fd(s);
    if (lo_fd < 0) {
	fprintf(stderr,"lo_server_get_socket_fd failed\n");
	perror("");
	exit(1);
    }
    struct timeval ts1,ts2;
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

	gettimeofday(&ts1,0);
	if (debug>2) {
	    printf("At select after %.2f msec.", (ts1.tv_usec-ts2.tv_usec)/1000.0+(ts1.tv_sec-ts2.tv_sec)*1000);
	    fflush(stdout);
	}
	struct timeval timeout;
	timeout.tv_usec=0;
	timeout.tv_sec=5;
	retval = select(maxfd + 1, &rfds, NULL, NULL, &timeout);
	gettimeofday(&ts2,0);
	if (debug>2) {
	    printf("Select done after %.2f msec. rfds=0x%lx\n", (ts2.tv_usec-ts1.tv_usec)/1000.0+(ts2.tv_sec-ts1.tv_sec)*1000,*(unsigned long*)&rfds);
	    fflush(stdout);
	}
	if (retval == -1) {
	    perror("select() error: ");
	    exit(1);
	} else if (retval == 0) {
	    fprintf(stderr,"Select timeout - restarting cameras\n");
	    for (int i=0;i<ncamera;i++) 
		cameras[i]->reset();
	} else {
	    // Process OSC messages
	    if (FD_ISSET(lo_fd, &rfds))
		// Process all queued messages
		while (lo_server_recv_noblock(s, 0) != 0)
		    if (doQuit)
			return;

	    // Read input from cameras that haven't got a frame yet
	    for (int i=0;i<ncamera;i++) {
		if (!cameras[i]->getFrame()->isValid() && cameras[i]->isRunning() && FD_ISSET(cameras[i]->getSocket(),&rfds)) {
		    //printf("1.Reading from  camera %d\n", cameras[i]->getID());
		    FD_CLR(cameras[i]->getSocket(),&rfds);
		    if (cameras[i]->read() < 0) {
			fprintf(stderr,"Error reading from camera %d, stopping acquisition\n", cameras[i]->getID());
			cameras[i]->reset();
		    }
		}
	    }

	    int haveAllFrames=1;
	    for (int i=0;i<ncamera;i++) {
		haveAllFrames=haveAllFrames && cameras[i]->getFrame()->isValid();
	    }

	    // Process if all the cameras have new frames
	    if (haveAllFrames) {
		processFrames();
	    } else {
		// Read any other queued cameras
		for (int i=0;i<ncamera;i++) {
		    if (cameras[i]->isRunning() && FD_ISSET(cameras[i]->getSocket(),&rfds)) {
			//printf("2.Reading from  camera %d\n", cameras[i]->getID());
			if (cameras[i]->read() < 0) {
			    fprintf(stderr,"Error reading from camera %d, restarting acquisition\n", cameras[i]->getID());
			    cameras[i]->reset();
			}
		    }
		}
	    }
	}
    }
    // NOT REACHED
}


void FrontEnd::processFrames() {
    const bool dumpVis=false;

    if (debug)
	printf("Processing frame %d\n",frame);
    double minus=1e99, maxus=0;
    for (int i=0;i<ncamera;i++) {
	Frame *frame=cameras[i]->getFrame();
	double us=frame->getStartTime().tv_usec/1e6 + frame->getStartTime().tv_sec;
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
	    if (vis[i]->processImage(frame,cameras[i]->getFPS())) {
		// Failed.  Skip processing this frame
		fprintf(stderr,"Processing of frame %d from camera %d failed; skipping\n", this->frame, i);
		cameras[i]->getFrame()->clearValid();
		return;
	    }

	    if (debug) {
		gettimeofday(&tend,NULL);
		printf("processImage() took %.3f msec.\n",(tend.tv_usec-tstart.tv_usec)/1000.0+(tend.tv_sec-tstart.tv_sec));
	    }
	    if (dumpVis) {
		printf("V[%d]=",i);
		for (int j=0;j<nled;j++)
		    printf("%d",vis[i]->getVisible()[j]);
		printf("\n");
	    }
	}
    }
    if (debug)
	printf("Frame jitter (based on first blocks received) = %.1f msec.\n", (maxus-minus)*1000);

    for (int i=0;i<ncamera;i++)
	cameras[i]->getFrame()->clearValid();   // Ready to rewrite

    sendMessages();
    frame=frame+1;
}

// Send all messages that are in the sendOnce list to the destinations
void FrontEnd::sendMessages() {
    if (debug>1)
	printf("sendMessages()  sendOnce=%ld, ndest=%d\n",sendOnce,dests.count());

    sendOnce |= sendAlways;
    for (int i=0;i<dests.count();i++) {
	char cbuf[10];
	sprintf(cbuf,"%d",dests.getPort(i));
	lo_address addr = lo_address_new(dests.getHost(i), cbuf);
	lo_send(addr,"/vis/beginframe","i",frame);
	for (int c=0;c<ncamera;c++) {
	    struct timeval ts=vis[c]->getTimestamp();
	    if (sendOnce & VIS) {
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
		lo_send(addr, "/vis/visible","iiiifb",c,cameras[c]->getCamFrameNum(),ts.tv_sec, ts.tv_usec, vis[c]->getCorrThresh(), data);
		lo_blob_free(data);
	    }
	    if (sendOnce & CORR) {
		// Send correlations
		if (debug>1)
		    printf("Sending CORR to %s:%d\n", dests.getHost(i),dests.getPort(i));
		lo_blob data = lo_blob_new(nled*sizeof(float),vis[c]->getCorr());
		lo_send(addr, "/vis/corr","iiiib",c,frame,ts.tv_sec, ts.tv_usec, data);
		lo_blob_free(data);
	    }
	    if (sendOnce & REFIMAGE) {
		if (debug)
		    printf("Sending REFIMAGE to %s:%d\n", dests.getHost(i),dests.getPort(i));
		const char *filename=vis[c]->saveRef();    // Save frame in file, retrieve filename (as float)
		if (filename!=0) {
		    printf("Saved image in %s\n", filename);
		    int width=vis[c]->getRefWidth();
		    int height=vis[c]->getRefHeight();
		    int depth=vis[c]->getRefDepth();
		    lo_send(addr, "/vis/refimage","iiiiiiiss",c,frame,ts.tv_sec,ts.tv_usec, width, height,depth,"f",filename);
		}
	    }	   
	    if (sendOnce & IMAGE) {
		if (debug)
		    printf("Sending IMAGE to %s:%d\n", dests.getHost(i),dests.getPort(i));
		Frame *fptr=cameras[c]->getFrame();
		const char *filename=fptr->saveImage();    // Save frame in file, retrieve filename (as bytes)
		if (filename!=0) {
		    printf("Saved image in %s\n", filename);
		    int width=fptr->getWidth();
		    int height=fptr->getHeight();
		    int depth=fptr->isColor()?3:1;
		    lo_send(addr, "/vis/image","iiiiiiiss",c,frame,ts.tv_sec,ts.tv_usec, width, height,depth,"b",filename);
		}
	    }
	}
	lo_send(addr,"/vis/endframe","i",frame);
	lo_address_free(addr);
    }
    sendOnce=0;
}


void FrontEnd::startStop(bool start) {
    printf("FrontEnd: %s\n", start?"start":"stop");
    for (int i=0;i<ncamera;i++)
	if (cameras[i]->startStop(start) < 0  && start) {
	    fprintf(stderr,"Failed to start camera %d, aborting startup\n", cameras[i]->getID());
	    start=false;
	    // Stop any cameras we already started
	    for (int j=0;j<=i;j++)
		(void)cameras[j]->startStop(false);
	    break;
	}

    // Send status update message
    for (int i=0;i<dests.count();i++) {
	char cbuf[10];
	sprintf(cbuf,"%d",dests.getPort(i));
	lo_address addr = lo_address_new(dests.getHost(i), cbuf);
	if (start)
	    lo_send(addr,"/vis/started","");
	else
	    lo_send(addr,"/vis/stopped","");
	lo_address_free(addr);
    }
}

// Get position of targets within images/refimages;   receive (0,0) is top left pixel of acquired images
void FrontEnd::setPos(int camera, int led, int xpos, int ypos, int twidth, int theight) {
    if (led==0)
	// Only print debug message once
	printf("setPos(%d,%d,%d,%d,%d,%d)\n",camera,led,xpos,ypos,twidth,theight);

    if (camera<0 || camera>=ncamera) {
	fprintf(stderr,"setPos: bad camera number: %d\n", camera);
	return;
    }
    if (led<0 || led>=nled) {
	fprintf(stderr,"setPos: bad led number: %d\n", led);
	return;
    }
    vis[camera]->setPosition(led,xpos,ypos,twidth,theight);
}

void FrontEnd::setFPS(int fps) {
    printf("Setting all cameras to %d FPS\n", fps);
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
    if (camera<0 || camera>=ncamera) {
	fprintf(stderr,"setRes: bad camera number: %d\n", camera);
	return;
    }
    cameras[camera]->setRes(res);
}

// Set ROI to use with camera,  x0=1,y0=1 starts at top left pixel of full frame
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
    unsigned int ntotal=0;
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

void FrontEnd::getStat(long int stat, int mode) {
    printf("getStat(%ld,%d)\n",stat,mode);fflush(stdout);
    if (mode==2)
	sendAlways |= stat;
    else
	sendAlways &= ~stat;

    if (mode>0)
	sendOnce |= stat;
}

void FrontEnd::addDest(const char *host, int port) {
    dests.add(host,port);
}

void FrontEnd::addDest(lo_message msg, int port) {
    char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
    addDest(host,port);
}

void FrontEnd::rmDest(const char *host, int port) {
    dests.remove(host,port);
}

void FrontEnd::rmDest(lo_message msg, int port) {
    char *host=lo_url_get_hostname(lo_address_get_url(lo_message_get_source(msg)));
    rmDest(host,port);
}

void FrontEnd::rmAllDest() {
    dests.removeAll();
}

void FrontEnd::ping(lo_message msg, int seqnum) {
    for (int i=0;i<dests.count();i++) {
	char cbuf[10];
	sprintf(cbuf,"%d",dests.getPort(i));
	lo_address addr = lo_address_new(dests.getHost(i), cbuf);
	lo_send(addr,"/ack","i",seqnum);
	lo_address_free(addr);
    }
}
