#ifndef _FRAME_H
#define _FRAME_H

#include <sys/time.h>
#include "defs.h"

// A frame of an MJPEG stream
class Frame {
    byte *frame;
    int allocLen;
    int frameLen;
    struct timeval timestamp;
    bool valid;
    
    // Decoded image
    int height, width;
    bool color;
    byte *image;
    int imageLen;
    int imageAllocLen;

 public:
    // Create a new frame from the provided data
    Frame();
    ~Frame();
    void copy(const byte *buffer,int buflen,const struct timeval &ts);
    // Save image in a temp file, return filename
    const char* saveFrame() const;   // Save frame as JPG
    const char* saveImage() const;  // Save decoded image as RAW
    bool isValid() const { return valid; }
    void clearValid() { valid=false; }
    void decompress();
    int getHeight() const { return height; }
    int getWidth() const { return width; }
    bool isColor() const { return color; }
    const byte *getImage() const { return image; }
    const struct timeval &getTimestamp() const { return timestamp; }
};


#endif
