#include <sstream>
#include <cairo.h>
#include <cairo-xlib.h>
#include "world.h"
#include "dbg.h"
#include "parameters.h"
#include "vis.h"

void World::initWindow() {
    dbg("World.initWindow",1) << "Creating thread for display..." << std::flush;
    surface=0;   // Until the thread gets rolling and sets it
    pthread_mutex_init(&displayMutex,NULL);
    int rc=pthread_create(&displayThread, NULL, runDisplay, (void *)this);
    if (rc) {
	fprintf(stderr,"pthread_create failed with error code %d\n", rc);
	exit(1);
    }
    dbgn("World.initWindow",1) << "done." << std::endl;
}

void *World::runDisplay(void *arg) {
    World *world=(World *)arg;
    SetDebug("pthread:Display");
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
	    break;
	}
	if (XPending(world->dpy)==0)
	    // Only redraw if there are no more messages
	    world->draw();
    }
}

// Draw info in given area (in pixels)
void World::drawinfo(cairo_t *cr, float left,  float top, float width, float height) const {
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
     msg << lastframe;
     cairo_set_font_size (cr, 40);
     cairo_show_text (cr, msg.str().c_str());
     cairo_set_font_size (cr, 10);

     for (unsigned int i=0;i<people.size();i++) {
	 cairo_move_to (cr, leftmargin, curline);curline+=lineskip;
	 msg.str("");
	 msg << people[i];
	 cairo_show_text (cr, msg.str().c_str());
     }

     cairo_restore(cr);
}

void World::draw(const Vis *vis) const {
    if (surface==NULL)
	return;
    
     dbg("World.draw",2) << "locking mutex" << std::endl;
     pthread_mutex_lock(&((World *)this)->displayMutex);
     dbg("World.draw",2) << "got mutex lock" << std::endl;

    cairo_surface_flush(surface);
    int width=cairo_xlib_surface_get_width(surface);
    int height=cairo_xlib_surface_get_height(surface);
     cairo_t *cr = cairo_create(surface);
     cairo_push_group(cr);

     // Erase surface
     cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
     cairo_paint(cr);

     // Draw info display
     drawinfo(cr,0.0,0.0,width/5.0,height);
     cairo_translate(cr,width/5.0,0.0);
     width=width-width/5.0;
     
     // Translate to center
     cairo_translate(cr,width/2.0,height/2.0);
     cairo_scale(cr,(float)width/(MAXRANGE*2),(float)height/MAXRANGE);
     float pixel=MAXRANGE*1.0/std::min(width,height);
     cairo_translate(cr,0,-(MAXRANGE/2.0));

     if  (drawRange && vis!=NULL) {
	 // Draw current range
	 const SickIO *s = vis->getSick();

	 float divergence = 0.011;    // 11 mrad of divergence
	 cairo_set_line_width(cr,1*pixel);
	 cairo_set_source_rgb (cr, 1.0,0.0,0.0);
	 const unsigned int *range = s->getRange(0);
	 for (unsigned int i=0;i<s->getNumMeasurements();i++) {
	     Point p1,p2;
	     float theta=s->getAngleRad(i);
	     p1.setThetaRange(theta+divergence/2,range[i]);
	     p2.setThetaRange(theta-divergence/2,range[i]);
	     cairo_move_to(cr, p1.X(), MAXRANGE-p1.Y());
	     cairo_line_to(cr, p2.X(), MAXRANGE-p2.Y());
	     cairo_stroke(cr);
	 }
     }

     // Draw background
     float scanRes = bg.getScanRes()*M_PI/180;
     float divergence = 0.011;    // 11 mrad of divergence
     cairo_set_line_width(cr,1*pixel);
     for (int k=0;k<2;k++) {
	 const std::vector<float> &range = bg.getRange(k);
	 const std::vector<float> &frac = bg.getFreq(k);
	 for (unsigned int i=0;i<range.size();i++) {
	     if (frac[i]>0.01) {
		 Point p1,p2;
		 float theta=(i-(range.size()-1)/2.0)*scanRes;
		 p1.setThetaRange(theta+divergence/2,range[i]);
		 p2.setThetaRange(theta-divergence/2,range[i]);
		 cairo_set_source_rgb (cr, frac[i],frac[i],frac[i]);
		 cairo_move_to(cr, p1.X(), MAXRANGE-p1.Y());
		 cairo_line_to(cr, p2.X(), MAXRANGE-p2.Y());
		 cairo_stroke(cr);
	     }
	 }
     }

     // Draw people
     cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
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

     cairo_pop_group_to_source(cr);  // Draw everything at once to avoid flicker
     cairo_paint(cr);
     cairo_destroy(cr);
     XFlush(dpy);
     dbg("World.draw",2) << "Unlocking mutex" << std::endl;
     pthread_mutex_unlock(&((World *)this)->displayMutex);
}
