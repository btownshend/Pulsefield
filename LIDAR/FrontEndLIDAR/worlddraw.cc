#include <sstream>
#include <cairo.h>
#include <cairo-xlib.h>
#include "world.h"
#include "dbg.h"
#include "parameters.h"

void World::initWindow() {
    dbg("World.initWindow",1) << "Creating thread for display..." << std::flush;
    surface=0;   // Until the thread gets rolling and sets it
    int rc=pthread_create(&displayThread, NULL, runDisplay, (void *)this);
    if (rc) {
	fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	exit(1);
    }
    dbgn("World.initWindow",1) << "done." << std::endl;
}

void *World::runDisplay(void *arg) {
    World *world=(World *)arg;
    dbg("World.runDisplay",1) << "Thread running" << std::endl;
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
	dbg("World.initWindow",8) << "Got event " << e.type << std::endl;

	switch (e.type) {
	case ConfigureNotify:
	    cairo_xlib_surface_set_size(world->surface,e.xconfigure.width, e.xconfigure.height);
	case MapNotify:
	case Expose:
	    world->draw();
	    break;
	}
    }
}

void World::draw() const {
    if (surface==NULL)
	return;
    cairo_surface_flush(surface);
    int width=cairo_xlib_surface_get_width(surface);
    int height=cairo_xlib_surface_get_height(surface);
     cairo_t *cr = cairo_create(surface);
     //     cairo_translate(cr,2.0,0.0);
     cairo_translate(cr,width/2.0,height/2.0);
     cairo_scale(cr,(float)width/(MAXRANGE*2),(float)height/MAXRANGE);
     float pixel=MAXRANGE*1.0/std::min(width,height);
     cairo_translate(cr,0,-(MAXRANGE/2.0));

     //    cairo_translate(cr,MAXRANGE,-MAXRANGE/2.0);
     cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
     cairo_paint(cr);
     cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
     cairo_set_font_size (cr, 10*pixel);
     cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
     cairo_move_to (cr, 0.0, MAXRANGE/2.0);
     std::ostringstream msg;
     msg << "Num people=" << people.size() << ", frame=" << lastframe;
     //printf("%s\n",msg.str().c_str());
     cairo_show_text (cr, msg.str().c_str());
     cairo_set_line_width(cr,2*pixel);
     for (std::vector<Person>::const_iterator p=people.begin();p!=people.end();p++){
	 if (p->getAge() >= AGETHRESHOLD) {
	     for (int i=0;i<2;i++) {
		 const Point &leg=p->getLeg(i).getPosition();
		 cairo_new_sub_path(cr);
		 cairo_arc(cr,leg.X(), MAXRANGE-leg.Y(),p->getLegStats().getDiam()/2.0,0.0,2*M_PI);
		 cairo_close_path(cr);
		 cairo_fill(cr);
	     }
	 }
     }
     cairo_show_page(cr);
     cairo_destroy(cr);
     XFlush(dpy);
}
