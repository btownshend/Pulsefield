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
    if (!XInitThreads()) {
	std::cerr << "Unable to set XLib to multithreaded operation" << std::endl;
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

void World::draw(int nsick, const SickIO * const * sick) const {
    if (surface==NULL)
	return;
    
    //dbg("World.draw",2) << "locking mutex" << std::endl;
     pthread_mutex_lock(&((World *)this)->displayMutex);
     //dbg("World.draw",2) << "got mutex lock" << std::endl;

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

     // Push to right side of screen if we the aspect ratio doesn't fit
     if (width>height*getWidth()/getHeight())
	 cairo_translate(cr,(width-height*getWidth()/getHeight())/2,0);

     // Scale to correct dimensions and flip Y axis, leaving a 5pixel margin
     float pixel=std::max(getWidth()/(width-10),getHeight()/(height-10));

     cairo_scale(cr,1/pixel,-1/pixel); 

     // Move center to center of active area
     Point center=getCenter();
     cairo_translate(cr,-center.X(),-center.Y());

     if  (drawRange && nsick>0) {
	 // Draw current range
	 cairo_set_line_width(cr,1*pixel);
	 for (int j=0;j<nsick;j++) {
	     cairo_set_source_rgb (cr, (j==0)?1.0:0.0,(j==1)?1.0:0.0,(j==2)?1.0:0.0);
	     const SickIO *s = sick[j];

	     const unsigned int *range = s->getRange(0);
	     for (unsigned int i=0;i<s->getNumMeasurements();i++) {
		 Point p1,p2;
		 float theta=s->getAngleRad(i);
		 float effDivergence=DIVERGENCE+EXITDIAMETER/range[i];
		 p1.setThetaRange(theta+effDivergence/2,range[i]);
		 p2.setThetaRange(theta-effDivergence/2,range[i]);
		 p1=s->localToWorld(p1);
		 p2=s->localToWorld(p2);
		 cairo_move_to(cr, p1.X(), p1.Y());
		 cairo_line_to(cr, p2.X(), p2.Y());
		 cairo_stroke(cr);
	     }
	     // Draw LIDAR position
	     cairo_move_to(cr,s->getOrigin().X(),s->getOrigin().Y());
	     cairo_arc(cr,s->getOrigin().X(),s->getOrigin().Y(),300.0f,s->getAngleRad(0)+s->getCoordinateRotationDeg()*M_PI/180+M_PI/2,s->getAngleRad(s->getNumMeasurements()-1)+s->getCoordinateRotationDeg()*M_PI/180+M_PI/2);
	     cairo_line_to(cr,s->getOrigin().X(),s->getOrigin().Y());
	     cairo_stroke(cr);

	     // Draw any targets
	     const std::vector<Point> tgts=s->getCalTargets();
	     for (int i=0;i<tgts.size();i++) {
		 Point p=s->localToWorld(tgts[i]*UNITSPERM);
		 //std::cout << "Unit " << j << " has " << tgts.size() << " cal points " << tgts[i] << " -> " << p << std::endl;
		 float len=300;
		 Point v1,v2;
		 v1.setThetaRange(j*M_PI/2/nsick,len);
		 v2.setThetaRange(j*M_PI/2/nsick+M_PI/2,len);
		 cairo_move_to(cr,p.X()+v1.X(), p.Y()+v1.Y());
		 cairo_line_to(cr,p.X()-v1.X(), p.Y()-v1.Y());
		 cairo_move_to(cr,p.X()+v2.X(), p.Y()+v2.Y());
		 cairo_line_to(cr,p.X()-v2.X(), p.Y()-v2.Y());
		 cairo_stroke(cr);
	     }
	 }
     }

     // Draw bounds
     cairo_set_line_width(cr,1*pixel);
     cairo_set_source_rgb (cr, 0.0,1.0,0.0);
     cairo_move_to(cr, getMinX(), getMinY());
     cairo_line_to(cr, getMinX(), getMaxY());
     cairo_line_to(cr, getMaxX(), getMaxY());
     cairo_line_to(cr, getMaxX(), getMinY());
     cairo_line_to(cr, getMinX(), getMinY());
     cairo_stroke(cr);

     // Draw people
     cairo_set_line_width(cr,2*pixel);
     for (int j=0;j<people.size();j++) {
	 if (people[j].getAge() >= AGETHRESHOLD) {
	     for (int i=0;i<2;i++) {
		 const Point &leg=people[j].getLeg(i).getPosition();
		 if (i==0)
		     cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
		 else
		     cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
		 cairo_new_sub_path(cr);
		 cairo_arc(cr,leg.X(), leg.Y(),people[j].getLeg(i).getDiam()/2.0,0.0,2*M_PI);
		 cairo_close_path(cr);
		 cairo_stroke(cr);
	     }
	 }
     }

     // Draw background
     bool drawBG=true;
     if (drawBG) {
	 for (int j=0;j<nsick;j++) {
	     cairo_set_line_width(cr,1*pixel);
	     for (int k=0;k<sick[j]->getBackground().numRanges();k++) {
		 const std::vector<float> &range = sick[j]->getBackground().getRange(k);
		 const std::vector<float> &sigma = sick[j]->getBackground().getSigma(k);
		 const std::vector<float> &frac = sick[j]->getBackground().getFreq(k);
		 for (unsigned int i=0;i<range.size();i++) {
		     if (frac[i]>0.01) {
			 Point p1,p2,p3,p4;
			 float theta=sick[j]->getBackground().getAngleRad(i);
			 float effDivergence=DIVERGENCE+EXITDIAMETER/range[i];
			 p1.setThetaRange(theta+effDivergence/2,range[i]-2*sigma[i]);
			 p2.setThetaRange(theta-effDivergence/2,range[i]-2*sigma[i]);
			 p3.setThetaRange(theta-effDivergence/2,range[i]+2*sigma[i]);
			 p4.setThetaRange(theta+effDivergence/2,range[i]+2*sigma[i]);
			 p1=sick[j]->localToWorld(p1);
			 p2=sick[j]->localToWorld(p2);
			 p3=sick[j]->localToWorld(p3);
			 p4=sick[j]->localToWorld(p4);
			 cairo_set_source_rgb (cr, frac[i],frac[i],frac[i]);
			 cairo_move_to(cr, p1.X(), p1.Y());
			 cairo_line_to(cr, p2.X(), p2.Y());
			 cairo_line_to(cr, p3.X(), p3.Y());
			 cairo_line_to(cr, p4.X(), p4.Y());
			 cairo_line_to(cr, p1.X(), p1.Y());
			 cairo_stroke(cr);
		     }
		 }
	     }
	 }
     }

     cairo_pop_group_to_source(cr);  // Draw everything at once to avoid flicker
     cairo_paint(cr);
     cairo_destroy(cr);
     XFlush(dpy);
     //dbg("World.draw",2) << "Unlocking mutex" << std::endl;
     pthread_mutex_unlock(&((World *)this)->displayMutex);
}
