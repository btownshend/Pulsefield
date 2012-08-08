#ifndef _CAMIO_H
#define _CAMIO_H

// Class for Camera I/O
// Requests an MJPEG stream and then receives it without blocking
// Separates sections and processes each one as they come in and then stores uncompressed image in frame

#include "frame.h"
#include "defs.h"


class CamIO {
    enum { HEADER, FRAMES } state;
    static const char *sep;
    char *servIP;
    int servPort;
    int id;
    int sock;	/* Socket for receiving data, -1 if not running */
    byte *frame;

    byte *buffer;     /* Buffer for receiving data */
    unsigned int buflen;      /* Length of buffer */
    unsigned int totalBytesRcvd;   /* Bytes read into buffer so far */

    Frame curFrame;   /* Current frame */
    int camFrameNum;

    // Camera request parameters
    bool halfRes;
    int quality;
    int fps;
    int roiX0, roiX1, roiY0, roiY1;   /* Region of interest */

    // Start time of current block (when first byte was received)
    struct timeval blockStartTime;
 private:
    // Find the first separator in the current buffer, or return NULL if not present    
    byte *split();
 public:
    CamIO(int _id, const char *host, int port);
    ~CamIO();

    void setROI(int x0, int y0, int x1, int y1);
    void setFPS(int _fps) { fps=_fps; reset(); }
    float getFPS() const { return fps; }

    void setRes(const char *res) { halfRes= (res[0]=='h' || res[0]=='H'); reset(); }

    // Open connection to camera, start reading stream;  return -1 for failure, 0 otherwise
    int open();

    // Close connection to camera
    int close();

    // Close and reopen connection using current settings
    int reset();

  // Get socket for reading from HTTP server (-1 if not running)
    int getSocket() const { return sock; } 

    // Read data from socket, store in internal buffer, return -1 for failure
    int read();

    Frame *getFrame() { return &curFrame; }

    int getX0() const { return roiX0; }
    int getX1() const { return roiX1; }
    int getY0() const { return roiY0; }
    int getY1() const { return roiY1; }

    int startStop(bool start);   // -1 if failure
    bool isRunning() const { return sock!=-1; }

    int getID() const { return id; }
};
#endif
