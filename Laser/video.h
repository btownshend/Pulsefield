#include <cairo-xlib.h>
#include "displaydevice.h"
#include "laser.h"

class Transform;

// Cross reference to locate points in window when mouse is clicked
class XRef {
 public:
    Laser *laser;
    int anchorNumber;
    bool dev;
    Point winpos;
    bool reset; 	// True to indicate that it should be moved to this position
    XRef(Laser *_laser, int _anchorNumber, bool _dev, Point _winpos) {laser=_laser; anchorNumber=_anchorNumber; dev=_dev; winpos=_winpos;  reset=false;}
};

class XRefs {
    std::vector<XRef> xref;
    int clickedEntry;
 public:
    XRefs() { clickedEntry=-1; }
    void markClosest(Point winpt);
    void update(Point newpos, bool clear);
    XRef *lookup(Laser *laser, int anchorNumber, bool dev);
    void push_back(const XRef &xr) { xref.push_back(xr); }
    void refresh(cairo_t *cr, Laser *laser, int anchorNumber, bool dev, Point pos);
    void clear() { clickedEntry=-1; xref.clear(); }
};

class Video: public DisplayDevice {
    // Support for screen drawing to monitor operation
    Display *dpy;
    cairo_surface_t *surface;
    pthread_t displayThread;
    static void *runDisplay(void *w);

    void drawDevice(cairo_t *cr, float left, float top, float width, float height, Laser *laser);
    void drawWorld(cairo_t *cr, float left, float top, float width, float height, Lasers &lasers);
    void drawText(cairo_t *cr, float left,  float top, float width, float height,const char *msg) const;
    void drawInfo(cairo_t *cr, float left,  float top, float width, float height) const;

    Lasers lasers;
    std::vector<Point> bounds;
    static XRefs xrefs;
    
    pthread_mutex_t mutex;
    int frame;
 public:
    // Local window routines
    Video(const Lasers &lasers);
    ~Video();

    int open();
    void update();
    void setBounds(const std::vector<Point> &_bounds) { bounds=_bounds; }
    void setFrame(int _frame) { frame=_frame;}
    void lock();
    void unlock();
    void save(std::ostream &s) const { lasers.saveTransforms(s); }
    void load(std::istream &s) { lasers.loadTransforms(s); }
};

