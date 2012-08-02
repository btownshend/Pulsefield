#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "frame.h"
#include "nanojpeg.h"

extern int debug;

Frame::Frame() {
    allocLen=0;
    frameLen=0;
    image = 0;
    valid=false;
    imageAllocLen=0;
    imageLen=0;
}

Frame::~Frame() {
    if (allocLen>0)
	delete [] frame;
    if (imageAllocLen>0)
	delete [] image;
}

int Frame::copy(const byte *buffer, int buflen,const struct timeval &ts) {
    if (buflen>allocLen) {
	// Reallocate storage
	if (allocLen>0)
	    delete [] frame;
	allocLen=buflen;
	frame=new byte[allocLen];
    }
    frameLen=buflen;
    memcpy(frame,buffer,frameLen);
    int elapsed = (ts.tv_usec - timestamp.tv_usec)/1000 + (ts.tv_sec-timestamp.tv_sec)*1000;
    if (debug)
	printf("Got frame of length %d at time %ld.%06ld (interframe time=%d msec):\n",frameLen,(long int)ts.tv_sec,(long int)ts.tv_usec,elapsed);
    if (valid)
	printf("Frame::copy: Overwriting valid frame that is %d msecs old\n",elapsed);
    timestamp=ts;
    return decompress();
}

const char *Frame::saveFrame() const {
    static char filename[1000];
    sprintf(filename,"/tmp/frame.%ld.%06ld.jpg",(long int)timestamp.tv_sec,(long int)timestamp.tv_usec);
    int fd=open(filename,O_CREAT|O_TRUNC|O_WRONLY,0644);
    if (write(fd,frame,frameLen)!=frameLen)  {
	perror(filename);
	close(fd);
	return (char *)0;
    }
    close(fd);
    return filename;
}

const char *Frame::saveImage() const {
    static char filename[1000];
    sprintf(filename,"/tmp/image.%ld.%06ld.raw",(long int)timestamp.tv_sec,(long int)timestamp.tv_usec);
    int fd=open(filename,O_CREAT|O_TRUNC|O_WRONLY,0644);
    assert(imageLen>0);
    if (write(fd,image,imageLen*sizeof(*image)) != (int)(imageLen*sizeof(*image)))  {
	perror(filename);
	close(fd);
	return (char *)0;
    }
    close(fd);
    return filename;
}

int Frame::decompress() {
    njInit();
    nj_result_t res=njDecode(frame,frameLen);
    if (res != NJ_OK) {
	fprintf(stderr,"Failed decode of JPEG image (error %d)\n", res);
	return -1;
    }
    width=njGetWidth();
    height=njGetHeight();
    color=njIsColor();
    imageLen=njGetImageSize();
    if (imageLen>imageAllocLen) {
	// Reallocate or allocate storage for image
	if (imageAllocLen>0)
	    delete [] image;
	imageAllocLen=imageLen;
	image = new byte[imageAllocLen];
    }
    memmove(image,njGetImage(),imageLen);
    njDone();
    valid=true;
    return 0;
}
