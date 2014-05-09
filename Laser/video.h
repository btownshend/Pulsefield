#include <cairo-xlib.h>
#include "displaydevice.h"
#include "laser.h"

class Transform;

class Video: public DisplayDevice {
    // Support for screen drawing to monitor operation
    Display *dpy;
    cairo_surface_t *surface;
    pthread_t displayThread;
    static void *runDisplay(void *w);

    void drawDevice(cairo_t *cr, float left, float top, float width, float height, const Laser *laser) const;
    void drawWorld(cairo_t *cr, float left, float top, float width, float height, const Lasers &lasers) const;
    void drawText(cairo_t *cr, float left,  float top, float width, float height,const char *msg) const;
    void drawInfo(cairo_t *cr, float left,  float top, float width, float height) const;

    Lasers lasers;
    std::vector<Point> bounds;
 public:
    // Local window routines
    Video(const Lasers &lasers);
    ~Video();

    int open();
    void update();
    void setBounds(const std::vector<Point> &_bounds) { bounds=_bounds; }
};

