#include <stdio.h>      /* for printf() and fprintf() */ 
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */ 
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */ 
#include <stdlib.h>     /* for atoi() and exit() */ 
#include <string.h>     /* for memset() */ 
#include <unistd.h>     /* for close() */ 
#include <assert.h>
#include <fcntl.h>
#include <sys/errno.h>
 
#include "camio.h"
#include "frame.h"

static const int RCVBUFSIZE=10*1024*1024;   /* Size of receive buffer */ 
const char *CamIO::sep = "--fbdr\r\n";
extern int errno;
extern int debug;

static void dumpdata(const byte *data, int len);
 
CamIO::CamIO(int _id, const char *_host, int _port) {
    id=_id;
    servIP=new char[strlen(_host)+1];
    strcpy(servIP,_host);
    servPort=_port;
    sock=-1;
    buffer=(byte *)0;
    buflen=0;
    roiX0=0; roiY0=1200; roiX1=9999; roiY1=1400;
    quality=15;
    fps=1;
    halfRes=false;
    // Initialize buffer
    buflen=RCVBUFSIZE;
    buffer = (byte *)new byte[buflen];
    if (buffer == (byte *)0) {
	fprintf(stderr, "CamIO: Unable to allocate %d bytes for buffer\n",buflen);
	exit(1);
    }
   camFrameNum=0;
   printf("Constructed camera %d at %s:%d\n", id, servIP, servPort);
};


CamIO::~CamIO() {
    if (isRunning())
	(void)close();
    if (buflen>0)
	delete buffer;
    delete [] servIP;
}

int CamIO::open() {
    /* Create a reliable, stream socket using TCP */ 
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        perror("socket() failed: "); 
	return -1;
    }
 
    /* Construct the server address structure */ 
    struct sockaddr_in servAddr; /* Echo server address */ 
    memset(&servAddr, 0, sizeof(servAddr));     /* Zero out structure */ 
    servAddr.sin_family      = AF_INET;             /* Internet address family */ 
    servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */ 
    servAddr.sin_port        = htons(80); /* Server port */ 
 
    /* Establish the connection to the echo server */ 
    printf("Opening connection to camera %d at %s...",id,servIP); fflush(stdout);
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)  {
        perror("connect() failed"); 
	return -1;
    }
    printf("done\n");

    const int MAXREQUEST = 1000;
    char request[MAXREQUEST];
    if (halfRes)
	// ROI sent to camera is still in terms of full res
	sprintf(request,"GET /mjpeg?res=half&x0=%d&y0=%d&x1=%d&y1=%d&quality=%d&doublescan=0&fps=%d HTTP/1.1\r\n\r\n",
		roiX0*2-1, roiY0*2-1, roiX1*2-1, roiY1*2-1, quality,fps);
    else
	sprintf(request,"GET /mjpeg?res=full&x0=%d&y0=%d&x1=%d&y1=%d&quality=%d&doublescan=0&fps=%d HTTP/1.1\r\n\r\n",
		roiX0, roiY0, roiX1, roiY1, quality,fps);

    int reqlen = strlen(request);          /* Determine input length */ 
    assert(reqlen+1<MAXREQUEST);

    /* Send the string to the server */ 
    printf("Sending request to camera %d: %s", id, request); fflush(stdout);
    if (send(sock, request, reqlen, 0) != reqlen)  {
        perror("send() sent a different number of bytes than expected"); 
	return -1;
    }
    printf("request accepted\n");

    totalBytesRcvd = 0;   // Empty

    /* Set the socket to non-blocking to read the responses */
    if (fcntl(sock,F_SETFL,O_NONBLOCK)<0) {
	perror("fcntl(NONBLOCK): ");
	return -1;
    }

    state = HEADER;

    return 0;
}

int CamIO::close() {
    if (sock==-1)
	return 0;
    if (::close(sock)<0) {
	perror("Socket close: ");
	return -1;
    }
    sock=-1;
    curFrame.clearValid();
    return 0;
}

int CamIO::reset() {
    if (isRunning()) {
	if (close() < 0)
	    return -1;
	if (open() < 0)
	    return -1;
    }
    return 0;
}

void CamIO::setROI(int x0, int y0, int x1, int y1) {
    roiX0=x0;
    roiY0=y0;
    roiX1=x1;
    roiY1=y1;
    reset();
}

// Find the first separator in the current buffer, or return NULL if not present    
byte *CamIO::split() {
    for (unsigned int i=0;i+strlen(sep)<=totalBytesRcvd;i++)
	if (memcmp(&buffer[i],sep,strlen(sep))==0) {
	    /// printf("Camera %d found separator at position %d\n", id, i);
	    return &buffer[i];
	}
    return NULL;
}

int CamIO::read() {
    if (!isRunning())
	return -1;

    while (1) {
	/* Receive the data from the camera */ 
        /* Receive up to the buffer size bytes from the sender */ 
	struct timeval ts;
	gettimeofday(&ts,0);
	int bytesRcvd = recv(sock, &buffer[totalBytesRcvd], buflen-totalBytesRcvd, 0);
	//	printf("Received: %d bytes from camera %d at %ld.%06ld, last bytes=%02x%02x%02x%02x\n",bytesRcvd, id,(long int)ts.tv_sec,(long int)ts.tv_usec,
	//       buffer[totalBytesRcvd+bytesRcvd-4],buffer[totalBytesRcvd+bytesRcvd-3],buffer[totalBytesRcvd+bytesRcvd-2],buffer[totalBytesRcvd+bytesRcvd-1]);
        if (bytesRcvd < 0)  {
	    if (errno==EAGAIN) 
		break;  // Would block
	    printf("errno=%d\n", errno);fflush(stdout);
            perror("recv() failed or connection closed prematurely"); 
	    return -1;
	} else if (bytesRcvd == 0)  {
	    if (totalBytesRcvd==0) {
		// EOF and finished reading everything there way
		fprintf(stderr,"Connection to camera %d closed\n", id);
		return -1;
	    }
	    break; 
	} else if (totalBytesRcvd==0) {
	    // Received some data at beginning of new block, log time
	    blockStartTime = ts;
	}
        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */ 
 
	byte *splitPos = split();
	if (splitPos== NULL) 
	    break;
	// Found MIME separator, splitPos is at beginning of separator
	int len=splitPos-buffer;
	if (state == HEADER) {
	    printf("Got header from camera %d:\n<",id);
	    fwrite(buffer,len,1,stdout);
	    printf(">\n");
	    state = FRAMES;
	} else {
	    const char *frameHeader = "Content-Type: image/jpeg\r\n\r\n";
	    if (memcmp(buffer,frameHeader,strlen(frameHeader))==0) {
		// matched expected frame header 
		byte *frameStart =buffer+strlen(frameHeader);
		int frameLen = len-strlen(frameHeader);
		if (debug)
		    printf("Camera %d: ", id);
		if (curFrame.copy(frameStart,frameLen,blockStartTime,id,camFrameNum) < 0) {
		    fprintf(stderr,"curFrame.copy() failed\n");
		    return -1;
		}
		camFrameNum++;
	    } else {
		fprintf(stderr,"Bad multipart frame of length %d:\n", len);
		dumpdata(buffer,len);
		return -1;
	    }
	}
	totalBytesRcvd -= (len+strlen(sep));
	if (totalBytesRcvd > 0) {
	    // copy rest of buffer
	    memmove(buffer,splitPos+strlen(sep),totalBytesRcvd);
	    // Still using old blockStartTime
	    printf("Camera %d has pre-read %d bytes, timestamp may be off\n", id, totalBytesRcvd);
	    blockStartTime=ts;   // Set it to the time of the last block read
	}

	if (totalBytesRcvd == buflen) {
	    fprintf(stderr,"Buffer for camera %d is full, flushing\n",id);
	    totalBytesRcvd=0;
	}
    }
    return 0;
}

static void dumpdata(const byte *buffer, int len) {
    for (int j=0;j<len &&j<64;j+=16) {
	for (int i=0;i<16&&i+j<len;i++)
	    fprintf(stderr,"%02x ",buffer[i+j]);
	fprintf(stderr,"  ");
	for (int i=0;i<16&&i+j<len;i++)
	    if (buffer[i+j]>=' ' && buffer[i+j]<0x7f)
		fprintf(stderr,"%c ",buffer[i+j]);
	    else
		fprintf(stderr,". ");
	fprintf(stderr,"\n");
    }
}

int  CamIO::startStop(bool start) {
    if (start && !isRunning()) {
	if (open() < 0)
	    return -1;
    } else if (!start && isRunning()) {
	if (close() < 0)
	    return -1;
    }
    return 0;
}
