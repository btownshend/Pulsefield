#include <cairo-xlib.h>
#include "displaydevice.h"

class Transform;

class Video: public DisplayDevice {
    // Support for screen drawing to monitor operation
    Display *dpy;
    cairo_surface_t *surface;
    pthread_t displayThread;
    static void *runDisplay(void *w);

    void drawDevice(cairo_t *cr, float left, float top, float width, float height, const std::vector<etherdream_point> &points,int unit) const;
    void drawWorld(cairo_t *cr, float left, float top, float width, float height, const std::vector<etherdream_point> &points, const Transform &transform) const;
    void drawText(cairo_t *cr, float left,  float top, float width, float height,const char *msg) const;
    void drawInfo(cairo_t *cr, float left,  float top, float width, float height) const;
 public:
    // Local window routines
    Video();
    ~Video();

    int open();
    void update(const std::vector<etherdream_point> &points, const Transform &transform);
    void update(const std::vector<etherdream_point> &points);
};

