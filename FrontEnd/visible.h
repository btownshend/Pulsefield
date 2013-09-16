// Support class for calculating LED visibility
#include "frame.h"

class Visible {
    static float updateTimeConstant, corrThreshold; 
    int nleds;
    int *xpos;   // Position of top left of each target region
    int *ypos;
    int *tgtWidth, *tgtHeight; // Width(x) and Height
    byte *visible;
    int *blockedframes, *visframes;   // Count of how many frames a particular pixel has been blocked
    byte *enabled;
    float *corr;
    // Reference image (full size) - subparts used for targets
    int refHeight, refWidth, refDepth;  
    float *refImage;
    struct timeval timestamp;  // Timestamp of image
    int camid;   // For debug messages
    void updateTarget(const Frame *frame,float fps);
 public:
    Visible(int nleds, int camid);
    ~Visible();
    int processImage(const Frame *frame, float fps);   // Process an image frame to determine corr, visibility; update reference.  Return 0 if success, -1 if failed (bad image?)

    void setPosition(int led, int xpos, int ypos, int tgtWidth,int tgtHeight);   // Set position (center point) of an LED target within frame images  (0,0) origin; use -1 if unused
    void setRefImage(int width, int height, int depth, const float *image=0);   // Set expected image -- should match size of received frames
    const byte *getVisible() const { return visible; }
    const float *getCorr() const { return corr; }
    int getRefHeight() const { return refHeight; }
    int getRefWidth() const { return refWidth; }
    int getRefDepth() const { return refDepth; }
    const struct timeval &getTimestamp() const { return timestamp; }

    const float *getRefImage() const { return refImage; }
    const char* saveRef(int c) const;
    
    static void setUpdateTimeConstant(float t) { updateTimeConstant=t; }
    static void setCorrThresh(float t) { corrThreshold=t; }
    static float getCorrThresh() { return corrThreshold; }
};
