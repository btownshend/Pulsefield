// Process visibility by computing cross correlations
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "visible.h"

float Visible::updateTimeConstant=60;  // Default is 60sec time constant for updating reference
float Visible::corrThreshold = 0.7;

Visible::Visible(int _nleds) {
    nleds=_nleds;
    xpos=new int[nleds];
    ypos=new int[nleds];
    visible=new byte[nleds];
    corr = new float[nleds];
    tgtWidth=new int[nleds];
    tgtHeight=new int[nleds];
    refImage = 0;

    // Default the values of xpos, ypos
    for (int i=0;i<nleds;i++) {
	xpos[i]=0;
	ypos[i]=0;
    }
}

Visible::~Visible() {
    delete [] xpos;
    delete [] ypos;
    delete [] tgtWidth;
    delete [] tgtHeight;
    delete [] visible;
    delete [] corr;
    if (refImage)
	delete [] refImage;
}

// Set position (center point) of an LED target within frame images  (0,0) origin; use -1 if unused
void Visible::setPosition(int led, int _x0, int _y0, int _tgtWidth, int _tgtHeight) {
    xpos[led]=_x0;
    ypos[led]=_y0;
    tgtWidth[led]=_tgtWidth;
    tgtHeight[led]=_tgtHeight;
}

// Set expected image -- should match size of received frames
void Visible::setRefImage(int width, int height, int depth, const float *image) {
    printf("setRefImage(%d,%d,%s)\n", width, height,image==0?"0":"image");
    refWidth=width;
    refHeight=height;
    refDepth=depth;
    if (refImage)
	delete [] refImage;
    refImage = new float[width*height*depth];
    for (int i=0;i<width*height*depth;i++)
	if (image)
	    refImage[i]=image[i];
	else
	    refImage[i] = 0.5;
}

// Process an image frame to determine visibility; fills visiblity array
void Visible::processImage(const Frame *frame, float fps) {
    assert(frame->getWidth() == refWidth);
    assert(frame->getHeight() == refHeight);
    assert(frame->isColor());
    const byte *fimg=frame->getImage();
    int nPixels = refWidth*refHeight;   // Number of pixels in each plane of image
    for (int i=0;i<nleds;i++) {
	int N=tgtWidth[i]*tgtHeight[i];
	if (xpos[i]<0 || ypos[i]<0) {
	    corr[i]=nan("?");
	    visible[i]=2;
	    continue;
	}
	if (xpos[i] >= frame->getWidth() || ypos[i] >= frame->getHeight()) {
	    fprintf(stderr,"Bad target position (%d,%d) with image of size (%d,%d)\n", xpos[i], ypos[i], frame->getWidth(), frame->getHeight());
	    corr[i]=nan("?");
	    visible[i]=2;
	    continue;
	}
	if (refDepth==1) {
	    // Grayscale match, used weighted sum of signal
	    float sxx=0,sxy=0,syy=0,sx=0,sy=0;
	    for (int h=0;h<tgtHeight[i];h++) {
		int index=(ypos[i]+h)*refWidth+xpos[i];
		const byte *x=&fimg[index*3];
		const float *y=&refImage[index];
		for (int w=0;w<tgtWidth[i];w++) {
		    float xv;
		    xv=2*float(x[0])+float(x[1])+float(x[2]);  // Convert to gray scale with extra weight to reds
		    sxx+=xv*xv;
		    sx+=xv;
		    sy+=(*y);
		    syy+=(*y)*(*y);
		    sxy+=xv*(*y++);
		    x+=3;
		}
	    }
	    float denom=sqrt((sxx-sx*sx/N)*(syy-sy*sy/N));
	    if (denom==0.0)
		corr[i]=0;
	    else
		corr[i]=(sxy-sx*sy/N)/denom;
	} else {
	    assert(refDepth==3);
	    // RGB, check correlation on each plane separately and use the max found
	    corr[i]=-2;   // Assume minimum possible (but correct this below if not increased)
	    for (int col=0;col<refDepth;col++) {
		float sxx=0,sxy=0,syy=0,sx=0,sy=0;
		for (int h=0;h<tgtHeight[i];h++) {
		    int index=(ypos[i]+h)*refWidth+xpos[i];
		    const byte *x=&fimg[index*3+col];   // Offset by color
		    const float *y=&refImage[index*3+col]; 
		    for (int w=0;w<tgtWidth[i];w++) {
			float xv;
			xv=float(x[0]);  // Just one color at a time
			sxx+=xv*xv;
			sx+=xv;
			sy+=(*y);
			syy+=(*y)*(*y);
			sxy+=xv*(*y);
			x+=3;  // Skip to next pixel of same color
			y+=3;
		    }
		}
		float denom=sqrt((sxx-sx*sx/N)*(syy-sy*sy/N));
		if (denom!=0.0) {
		    float corrcol=(sxy-sx*sy/N)/denom;
		    if (corrcol > corr[i])
			corr[i]=corrcol;
		    if (i==-1) {
			int tlind=ypos[i]*refWidth+xpos[i];
			printf("tl=(%d,%d), IM=%d, REF=%f, Corr(%d)=%.3f\n",xpos[i],ypos[i],fimg[tlind*3+col], refImage[tlind*3+col], col, corrcol);
			tlind=(ypos[i]+tgtHeight[i]-1)*refWidth+(xpos[i]+tgtWidth[i]-1);
			printf("br=(%d,%d), IM=%d, REF=%f, Corr(%d)=%.3f\n",xpos[i]+tgtWidth[i]-1,ypos[i]+tgtHeight[i]-1,fimg[tlind*3+col], refImage[tlind*3+col], col, corrcol);
		    }
		}
	    }
	    if (corr[i]==-2)
		corr[i]=0.0;   // Since we used -2 as the initializer before
	}
	if(corr[i]<-1.01 || corr[i]>1.01)
	    fprintf(stderr,"Warning: corr[%d]=%f\n", i, corr[i]);
	visible[i]=corr[i]>=corrThreshold;

    }
    updateTarget(frame,fps);

    timestamp=frame->getTimestamp();
}

// Update target by adding weight*currImage to (1-weight)*currTarget
void Visible::updateTarget(const Frame *frame, float fps) {
    assert(frame->getWidth() == refWidth);
    assert(frame->getHeight() == refHeight);
    assert(frame->isColor());

    if (updateTimeConstant == 0)
	// Special case, no updates
	return;

    float weight = 1.0/(updateTimeConstant * fps);   

    const byte *newimg = frame->getImage();
    float *refimg = refImage;
    float oweight = 1.0-weight;
    int nPixels = refHeight*refWidth;
    const byte *endimg = &newimg[nPixels*3];
    float scale=weight*refDepth/3/255;
    if (refDepth==1)
	while (newimg<endimg) {
	    *refimg++ = (*refimg)*oweight+(newimg[0]+newimg[1]+newimg[2])*scale;
	    newimg+=3;
	}
    else
	while (newimg<endimg) {
	    *refimg++ = (*refimg)*oweight+(*newimg)*scale;
	    newimg++;
	}
}

// Save reference image in a file
const char* Visible::saveRef() const {
    static char filename[1000];
    
    sprintf(filename,"/tmp/refimage.%ld.%06ld.raw",(long int)timestamp.tv_sec,(long int)timestamp.tv_usec);
    int fd=open(filename,O_CREAT|O_TRUNC|O_WRONLY,0644);
    int nbytes=refHeight*refWidth*refDepth*sizeof(*refImage);
    if (write(fd,refImage,nbytes)!=nbytes)  {
	perror(filename);
	exit(1);
    }
    close(fd);
    return filename;
}
