#include <cairo-xlib.h>
#include "displaydevice.h"
#include "lasers.h"

class Transform;
class Video;

// Cross reference to locate points in window when mouse is clicked
class XRef {
 public:
    std::shared_ptr<Laser> laser;
    int anchorNumber;
    bool dev;
    Point winpos;
    bool reset; 	// True to indicate that it should be moved to this position
    XRef(std::shared_ptr<Laser> _laser, int _anchorNumber, bool _dev, Point _winpos) {laser=_laser; anchorNumber=_anchorNumber; dev=_dev; winpos=_winpos;  reset=false;}
};

class XRefs {
    std::vector<XRef> xref;
    int clickedEntry;
 public:
    XRefs() { clickedEntry=-1; }
    void markClosest(Point winpt);
    void update(Point newpos, bool clear);
    XRef *lookup(std::shared_ptr<Laser>laser, int anchorNumber, bool dev);
    void push_back(const XRef &xr) { xref.push_back(xr); }
    void refresh(cairo_t *cr, std::shared_ptr<Laser>laser, Video &video, int anchorNumber, bool dev, Point pos);
    void clear() { clickedEntry=-1; xref.clear(); }
};

class Video: public DisplayDevice {
    // Support for screen drawing to monitor operation
    Display *dpy;
    Window window;

    cairo_surface_t *surface;
    pthread_t displayThread;
    static void *runDisplay(void *w);

    void drawDevice(cairo_t *cr, float left, float top, float width, float height, std::shared_ptr<Laser>laser);
    void drawWorld(cairo_t *cr, float left, float top, float width, float height);
    void drawText(cairo_t *cr, float left,  float top, float width, float height,const char *msg) const;
    void drawInfo(cairo_t *cr, float left,  float top, float width, float height) const;

    std::shared_ptr<Lasers> lasers;
    std::vector<Point> bounds;  // Polygonal bounds of active area
    float minLeft, maxRight, minBottom, maxTop;   // Bounding box
    static XRefs xrefs;
    
    // Locking
    pthread_mutex_t mutex;
    void lock();
    void unlock();
    
    std::ostringstream msg; // Message for display in bottom of window
    bool dirty;
    void update();
    void save(std::ostream &s) const { lasers->saveTransforms(s); }
    void load(std::istream &s) { lasers->loadTransforms(s); }
 public:
    // Local window routines
    Video(std::shared_ptr<Lasers> lasers);
    ~Video();

    int open();
    void setBounds(const std::vector<Point> &_bounds);

    Point constrainPoint(Point p) const;
    std::ostream &newMessage() { msg.str(""); return msg; }

    // Display needs refresh
    void setDirty();
};

