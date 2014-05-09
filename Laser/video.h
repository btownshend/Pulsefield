#include <cairo-xlib.h>
#include "displaydevice.h"

class Video: public DisplayDevice {
    // Support for screen drawing to monitor operation
    Display *dpy;
    cairo_surface_t *surface;
    pthread_t displayThread;
    static void *runDisplay(void *w);
 public:
    // Local window routines
    Video();
    ~Video();

    int open();
    void update(const std::vector<etherdream_point> points);
    void drawinfo(cairo_t *cr, float left,  float top, float width, float height) const;
};

