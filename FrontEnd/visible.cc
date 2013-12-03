// Process visibility by computing cross correlations
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "visible.h"

float Visible::updateTimeConstant=60;  // Default is 60sec time constant for updating reference
float Visible::corrThreshold = 0.7;
bool Visible::fgDetector = false;
float Visible::fgMinVar = (2.0/255)*(2.0/255); 
float Visible::fgMaxVar = (10.0/255)*(10.0/255);
float Visible::fgThresh[2] = {1.5,2.0};  // Thresholds for fg/bg detector
float Visible::fgScale = 4.0;  // fg/bg scaling

Visible::Visible(int _nleds, int _camid) {
    nleds=_nleds;
    camid=_camid;
    xpos=new int[nleds];
    ypos=new int[nleds];
    visible=new byte[nleds];
    blockedframes=new int[nleds];
    visframes=new int[nleds];
    enabled=new byte[nleds];
    corr = new float[nleds];
    tgtWidth=new int[nleds];
    tgtHeight=new int[nleds];
    refImage = 0;
    refImage2 = 0;

    // Default the values of xpos, ypos
    for (int i=0;i<nleds;i++) {
	xpos[i]=0;
	ypos[i]=0;
	blockedframes[i]=0;
	visframes[i]=0;
	enabled[i]=1;
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
    if (refImage2)
	delete [] refImage2;
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
    if (refImage2)
	delete [] refImage2;
    refImage = new float[width*height*depth];
    refImage2 = new float[width*height*depth];
    for (int i=0;i<width*height*depth;i++) {
	if (image)
	    refImage[i]=image[i];
	else
	    refImage[i] = 0.5;
	refImage2[i]=3.0/255;    // Estimate of variance
    }
}

// Process an image frame to determine visibility; fills visiblity array
int Visible::processImage(const Frame *frame, float fps) {
    assert(frame->getWidth() == refWidth);
    assert(frame->getHeight() == refHeight);
    assert(frame->isColor());
    const byte *fimg=frame->getImage();
    int totalvis=0;   // Count of number of visible pixels
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
	    const float scale=1.0/(255*4);
	    if (fgDetector) {
		float svar=0;
		int cnt=0;
		for (int h=0;h<tgtHeight[i];h++) {
		    int index=(ypos[i]+h)*refWidth+xpos[i];
		    const byte *x=&fimg[index*3];
		    const float *y=&refImage[index];
		    const float *var=&refImage2[index];
		    for (int w=0;w<tgtWidth[i];w++) {
			float xv=(2*float(x[0])+float(x[1])+float(x[2]))*scale;  // Convert to gray scale with extra weight to reds
			float v=*var;
			if (v<=fgMaxVar) {
			    if (v<fgMinVar) v=fgMinVar;
			    svar+=(xv-*y)*(xv-*y)/v;
			    cnt++;
			}
			var++;
			x+=3;
			y++;
		    }
		}
		svar=sqrt(svar/cnt);
		corr[i]=1.0-svar/fgScale;   // Make it look like a correlation roughly so thresholding will work correctly
		visible[i]=(svar<fgThresh[0])?1:((svar>fgThresh[1])?0:2);
	    } else {
		// Correlation based
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
		visible[i]=corr[i]>=corrThreshold;
	    }
	} else {
	    assert(refDepth==3);
	    // RGB, check correlation on each plane separately and use the max found
	    if (fgDetector) {
		const float scale=1.0/255;
		float svar=0;
		int cnt=0;
		for (int col=0;col<refDepth;col++) {
		    for (int h=0;h<tgtHeight[i];h++) {
			int index=(ypos[i]+h)*refWidth+xpos[i];
			const byte *x=&fimg[index*3+col];   // Offset by color
			const float *y=&refImage[index*3+col]; 
			const float *var=&refImage2[index*3+col];
			for (int w=0;w<tgtWidth[i];w++) {
			    float xv=float(x[0])*scale;  // Just one color at a time
			    float v=*var;
			    if (v<=fgMaxVar) {
				if (v<fgMinVar) v=fgMinVar;
				svar+=(xv-*y)*(xv-*y)/v;
				cnt++;
			    }
				//if (i==100 && w==1 && h==1)
				//	printf("xpos=%d, ypos=%d, y2=%g, y=%g, x=%d, xv=%g, var=%g,svar=%g,xv-y=%g, (xv-y)^2/var=%g\n",xpos[i],ypos[i],*y2,*y,x[0],xv,var,svar,xv-*y,(xv-*y)*(xv-*y)/var);
			    x+=3;  // Skip to next pixel of same color
			    y+=3;
			    var+=3;
			}
		    }
		    //  if (i==100) {
		    //	printf("Col=%d, svar=%g\n",col,svar);
		    //}
		}
		svar=sqrt(svar/cnt);
		corr[i] = 1.0-svar/fgScale;
		visible[i]=(svar<fgThresh[0])?1:((svar>fgThresh[1])?0:2);
	    } else {
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
		    float denom2=(sxx-sx*sx/N)*(syy-sy*sy/N);
		    float denom;
		    if (denom2>=0) 
			denom=sqrt(denom2);
		    else {
			fprintf(stderr,"Warning denom[%d]=%f (<0)\n",i,denom2);
			denom=0;
		    }
		    if (denom!=0.0) {
			float corrcol=(sxy-sx*sy/N)/denom;
			if (corrcol > corr[i])
			    corr[i]=corrcol;
		    }
		}
		if (corr[i]==-2)
		    corr[i]=0.0;   // Since we used -2 as the initializer before
		visible[i]=corr[i]>=corrThreshold;
	    }
	}
	if (i==-1) {
	    // Debug
	    int tlind=ypos[i]*refWidth+xpos[i];
	    for (int col=0;col<refDepth;col++) {
		printf("tl=(%d,%d), IM=%d, REF=%f, REF2=%f, Corr(%d)=%g\n",xpos[i],ypos[i],fimg[tlind*3+col], refImage[tlind*refDepth+col], refImage2[tlind*refDepth+col],col,corr[i]);
		tlind=(ypos[i]+tgtHeight[i]-1)*refWidth+(xpos[i]+tgtWidth[i]-1);
		printf("br=(%d,%d), IM=%d, REF=%f, REF2=%f, Corr(%d)=%g\n",xpos[i]+tgtWidth[i]-1,ypos[i]+tgtHeight[i]-1,fimg[tlind*3+col], refImage[tlind*refDepth+col], refImage2[tlind*refDepth+col], col,corr[i]);
	    }
	}
	if((corr[i]<-1.01 && ~fgDetector) || corr[i]>1.01)
	    fprintf(stderr,"Warning: corr[%d]=%f\n", i, corr[i]);
	// Need 10 visible frames in a row to enable, 100 blocked (not necessarily in a row) since last enable to disable
	if (visible[i]==1) {
	    visframes[i]++;
	    if (visframes[i]>400) {
		if (!enabled[i])
		    printf("Enabling ray %d.%d (had been blocked for %d frames)\n",camid,i,blockedframes[i]);
		enabled[i]=1;
		blockedframes[i]=0;
		visframes[i]=0;
	    }
	} else if (visible[i]==0) {
	     blockedframes[i]++;
	     visframes[i]=0;
	     if (blockedframes[i]>50 && enabled[i]) {
	 	enabled[i]=0;
	 	printf("Disabling ray %d.%d\n",camid,i);
	     }
	}
	if (!enabled[i]) {
	    visible[i]=2;
	    continue;
	}

	totalvis=totalvis+(visible[i]?1:0);
    }
    if (totalvis < 10) {
	fprintf(stderr,"Warning: Only %d visible pixels for frame; skipping\n",totalvis);
	//	return -1;
    }
    updateTarget(frame,fps);

    timestamp=frame->getStartTime();
    return 0;
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
    float *refimg2 = refImage2;
    float oweight = 1.0-weight;
    int nPixels = refHeight*refWidth;
    const byte *endimg = &newimg[nPixels*3];
    if (refDepth==1) {
	float scale=1/4.0/255.0;
	while (newimg<endimg) {
	    float xv=(2.0f*newimg[0]+newimg[1]+newimg[2])*scale;
	    float xvs2=(xv-*refimg)*(xv-*refimg);
	    *refimg = (*refimg)*oweight+xv*weight;
	    *refimg2 = (*refimg2)*oweight+xvs2*weight;   // Update variance
	    newimg+=3;
	    refimg++;
	    refimg2++;
	}
    } else {
	float scale=1.0/255;
	while (newimg<endimg) {
	    float xv=*newimg *scale;
	    float xvs2=(xv-*refimg)*(xv-*refimg);
	    *refimg = (*refimg)*oweight+xv*weight;
	    float oldval=*refimg2;
	    *refimg2 = (*refimg2)*oweight+xvs2*weight;
	    if (newimg==endimg-1)
		printf("newimg=%d, xv=%g, xvs2=%g, refimg=%g, refimg2=%g->%g, weight=%g\n", *newimg, xv, xvs2, *refimg, oldval, *refimg2, weight);
	    newimg++;
	    refimg++;
	    refimg2++;
	}
    }
}

// Save reference image in a file
const char* Visible::saveRef(int c) const {
    static char filename[1000];
    
    sprintf(filename,"/tmp/refimage.%d,%ld.%06ld.raw",c,(long int)timestamp.tv_sec,(long int)timestamp.tv_usec);
    int fd=open(filename,O_CREAT|O_TRUNC|O_WRONLY,0644);
    int nbytes=refHeight*refWidth*refDepth*sizeof(*refImage);
    if (write(fd,refImage,nbytes)!=nbytes)  {
	close(fd);
	perror(filename);
	return (char *)0;
    }
    close(fd);
    return filename;
}

const char* Visible::saveRef2(int c) const {
    if (!fgDetector)
	return (char *)0;
    static char filename[1000];
    sprintf(filename,"/tmp/refimage2.%d,%ld.%06ld.raw",c,(long int)timestamp.tv_sec,(long int)timestamp.tv_usec);
    int fd=open(filename,O_CREAT|O_TRUNC|O_WRONLY,0644);
    int nbytes=refHeight*refWidth*refDepth*sizeof(*refImage2);
    if (write(fd,refImage2,nbytes)!=nbytes)  {
	close(fd);
	perror(filename);
	return (char *)0;
    }
    close(fd);
    return filename;
}

void Visible::setFgDetector(bool on,float minvar, float maxvar, float fgscale,float fgthresh1,float fgthresh2) {
    fgDetector=on; fgMinVar=minvar; fgMaxVar=maxvar; fgScale=fgscale; fgThresh[0]=fgthresh1; fgThresh[1]=fgthresh2;
    printf("Set foreground/background detector to %d, var=[%g,%g], scale=%g, thresh=[%g,%g]\n", fgDetector,fgMinVar,fgMaxVar,fgScale,fgThresh[0],fgThresh[1]);
}
