#include <sstream>
#include <cairo.h>
#include <cairo-xlib.h>
#include "video.h"
#include "dbg.h"

Video::Video() {
    ;
}

Video::~Video() {
    ; // TODO
}

int Video::open() {
    dbg("Video.initWindow",1) << "Creating thread for display..." << std::flush;
    surface=0;   // Until the thread gets rolling and sets it
    int rc=pthread_create(&displayThread, NULL, runDisplay, (void *)this);
    if (rc) {
	fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	exit(1);
    }
    dbgn("Video.initWindow",1) << "done." << std::endl;
    return 0;
}

void *Video::runDisplay(void *arg) {
    Video *world=(Video *)arg;
    dbg("Video.runDisplay",1) << "Thread running" << std::endl;
    world->dpy = XOpenDisplay(NULL);
    if (world->dpy == NULL) {
	fprintf(stderr, "Error: Can't open display. Is DISPLAY set?\n");
	return NULL;
    }

    Window w;
    w = XCreateSimpleWindow(world->dpy, RootWindow(world->dpy, 0),0, 0, 800, 400, 0, 0, BlackPixel(world->dpy, 0));
    XSelectInput(world->dpy, w, StructureNotifyMask | ExposureMask);
    XMapWindow(world->dpy, w);

    world->surface = cairo_xlib_surface_create(world->dpy, w, DefaultVisual(world->dpy, 0), 800, 400);

    while (1) {
	XEvent e;
	XNextEvent(world->dpy, &e);
	dbg("Video.initWindow",8) << "Got event " << e.type << std::endl;

	switch (e.type) {
	case ConfigureNotify:
	    cairo_xlib_surface_set_size(world->surface,e.xconfigure.width, e.xconfigure.height);
	case MapNotify:
	case Expose:
	    //world->draw();
	    break;
	}
    }
}

// Draw info in given area (in pixels)
void Video::drawinfo(cairo_t *cr, float left,  float top, float width, float height) const {
    const float leftmargin=5+left;
    const float firstline=8+top;
    const float lineskip=15;
    float curline=firstline;

    cairo_save(cr);

    cairo_rectangle(cr,left,top,width,height);
    cairo_clip(cr);
    cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

     std::ostringstream msg;

     curline+=lineskip*3;
     cairo_move_to (cr, leftmargin, curline);curline+=lineskip;
     msg << "Test";
     cairo_set_font_size (cr, 40);
     cairo_show_text (cr, msg.str().c_str());
     cairo_set_font_size (cr, 10);

     for (unsigned int i=0;i<4;i++) {
	 cairo_move_to (cr, leftmargin, curline);curline+=lineskip;
	 msg.str("Line ");
	 msg << i;
	 cairo_show_text (cr, msg.str().c_str());
     }

     cairo_restore(cr);
}

void Video::update(const std::vector<etherdream_point> points) {
    if (surface==NULL)
	return;
    cairo_surface_flush(surface);
    int width=cairo_xlib_surface_get_width(surface);
    int height=cairo_xlib_surface_get_height(surface);
     cairo_t *cr = cairo_create(surface);

     // Erase surface
     cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
     cairo_paint(cr);

     // Draw info display
     drawinfo(cr,0.0,0.0,width/5.0,height);
     cairo_translate(cr,width/5.0,0.0);
     width=width-width/5.0;
     
     // Translate to center
     cairo_translate(cr,width/2.0,height/2.0);
     const int MAXVALUE=32767;
     cairo_scale(cr,(float)width/(MAXVALUE*2),(float)height/(MAXVALUE*2));
     float pixel=MAXVALUE*1.0/std::min(width,height);
     cairo_translate(cr,0,0);

     // Draw background
     cairo_set_line_width(cr,1*pixel);
     etherdream_point lastpt = points[points.size()-1];
     for (unsigned int i=0;i<points.size();i++) {
	 etherdream_point pt = points[i];
	 cairo_set_source_rgb (cr,pt.r/65535.0,pt.g/65535.0,pt.b/65535.0);
	 cairo_move_to(cr, lastpt.x,lastpt.y);
	 cairo_line_to(cr, pt.x, pt.y);
	 cairo_stroke(cr);
	 lastpt=pt;
     }

     cairo_show_page(cr);
     cairo_destroy(cr);
     XFlush(dpy);
}
