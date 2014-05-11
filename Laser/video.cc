#include <iostream>
#include <sstream>
#include <cairo.h>
#include <cairo-xlib.h>
#include "video.h"
#include "point.h"
#include "transform.h"
#include "dbg.h"

const int MAXVALUE=32767;
const float titleHeight=15;   // in pixels

Video::Video(const Lasers & _lasers): lasers(_lasers), bounds(4) {
    pthread_mutex_init(&mutex,NULL);
    // Default bounds for world view
    bounds[0]=Point(-6,0);
    bounds[1]=Point(6,0);
    bounds[2]=Point(6,6);
    bounds[3]=Point(-6,6);
}

Video::~Video() {
    pthread_mutex_destroy(&mutex);
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
    XSelectInput(world->dpy, w, StructureNotifyMask | ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    XMapWindow(world->dpy, w);

    world->surface = cairo_xlib_surface_create(world->dpy, w, DefaultVisual(world->dpy, 0), 800, 400);

    while (1) {
	XEvent e;
	
	dbg("Video.runDisplay",2) << "Event  wait for display " << world->dpy << std::endl;
	XNextEvent(world->dpy, &e);
	dbg("Video.runDisplay",2) << "Got event " << e.type << std::endl;
	world->lock();

	switch (e.type) {
	case KeyPress:
	    {
		const char *filename="transforms.save";
		KeySym key;
		const int bufsize=20;
		char buffer[bufsize];
		int charcount = XLookupString(&e.xkey,buffer,bufsize,&key,NULL);
		dbg("Video.runDisplay",2)  << "Key Pressed:  code=" << e.xkey.keycode << ", sym=" << key << ", keycount=" << charcount << std::endl;
		if (key==XK_s) {
		    dbg("Video.runDisplay",1) << "Saving transforms in " << filename << std::endl;
		    world->newMessage() << "Saved transforms in " << filename;
		    std::ofstream ofs(filename);
		    world->save(ofs);
		}
		if (key==XK_l) {
		    dbg("Video.runDisplay",1) << "Loading transforms from " << filename << std::endl;
		    world->newMessage() << "Loaded transforms from " << filename;
		    std::ifstream ifs(filename);
		    world->load(ifs);
		}
	    }
	    break;
	case ButtonPress:
	    std::cout << "Button Pressed:  " << e.xbutton.x << ", " << e.xbutton.y << std::endl;
	    xrefs.markClosest(Point(e.xbutton.x,e.xbutton.y));
	    break;
	case ButtonRelease:
	    std::cout << "Button Released:  " << e.xbutton.x << ", " << e.xbutton.y << std::endl;
	    xrefs.update(Point(e.xbutton.x,e.xbutton.y),true);
	    break;
	case MotionNotify:
	    std::cout << "Motion:  " << e.xmotion.x << ", " << e.xmotion.y << " with " << XPending(world->dpy) << " events queued" << std::endl;
	    xrefs.update(Point(e.xbutton.x,e.xbutton.y),false);
	    break;
	case ConfigureNotify:
	    cairo_xlib_surface_set_size(world->surface,e.xconfigure.width, e.xconfigure.height);
	    break;
	case MapNotify:
	case Expose:
	    break;
	}
	world->unlock();

	if (XPending(world->dpy)==0)
	    world->update();
    }
}

XRefs Video::xrefs;

// Mark the point closest to winpt for updating it (i.e. when mouse clicked)
void XRefs::markClosest(Point winpt) {
    if (clickedEntry!=-1) {
	dbg("XRefs.markClosest",1) << "Button already pressed, not re-marking" << std::endl;
	return;
    }
    float dist=1e99;
    for (unsigned int i=0;i<xref.size();i++)  {
	float d=(winpt-xref[i].winpos).norm();
	if (d<dist) {
	    dist=d;
	    clickedEntry=i;
	}
    }
    if (clickedEntry>=0) 
	dbg("XRefs.markClosest",1) << "Click at " << winpt << "; closest = " << clickedEntry << ": " << xref[clickedEntry].winpos << std::endl; 
}

XRef *XRefs::lookup(Laser *laser, int anchorNumber, bool dev) { 
    dbg("XRefs.lookup",3) << "lookup(" << laser->getUnit() << "," << anchorNumber << "," << dev << ")  N=" << xref.size() << " -> ";
    for (unsigned int i=0;i<xref.size();i++)  {
	XRef *x=&xref[i];
	if (x->laser==laser && x->anchorNumber==anchorNumber && x->dev==dev) {
	    dbgn("XRefs.lookup",3) << i << std::endl;
	    return x;
	}
    }
    dbgn("XRefs.lookup",3) << "NULL" << std::endl;
    return NULL;
}

void XRefs::update(Point newpos, bool clear) {
    if (clickedEntry<0) {
	dbg("Xrefs.update",1) << "update() with no clicked entry" << std::endl;
	return;
    }
    assert(clickedEntry<(int)xref.size());
    dbg("XRefs.update",1) << "Moved point " << clickedEntry << " from " << xref[clickedEntry].winpos << " to " << newpos << std::endl;
    xref[clickedEntry].winpos=newpos;
    xref[clickedEntry].reset=true;
    if (clear)
	clickedEntry=-1;
}

	     
// Update table with given xref and modify underlying Laser struct if reset is set
void XRefs::refresh(cairo_t *cr, Laser *laser, int anchorNumber, bool dev, Point pos) {
    XRef *entry=lookup(laser,anchorNumber,dev);
    if (entry!=NULL && entry->reset) {
	// Move point
	double wx=entry->winpos.X();
	double wy=entry->winpos.Y();
	cairo_device_to_user(cr,&wx,&wy);
	dbg("XRefs.refresh",1) << "Found xref entry; moving laser " << entry->laser->getUnit() << " anchor " << anchorNumber
			       << " to " << Point(wx,wy) << std::endl;
	if (entry->dev)
	    entry->laser->getTransform().setDevPoint(anchorNumber,Point(wx,wy));
	else
	    entry->laser->getTransform().setFloorPoint(anchorNumber,Point(wx,wy));
	entry->laser->getTransform().recompute();
	entry->reset=false;
    } else  {
	double wx=pos.X(), wy=pos.Y();
	cairo_user_to_device(cr,&wx,&wy);
	if (entry==NULL)
	    xref.push_back(XRef(laser,anchorNumber,dev,Point(wx,wy)));
	else
	    entry->winpos=Point(wx,wy);
    }
}

// Draw text
void Video::drawText(cairo_t *cr, float left,  float top, float width, float height,const char *msg) const {
    cairo_save(cr);
    cairo_translate(cr,left,top);
    cairo_rectangle(cr,0.0,0.0,width,height);
    cairo_clip(cr);

    cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
     cairo_set_font_size (cr, height*0.8);
     cairo_text_extents_t extents;
     cairo_text_extents(cr,msg,&extents);
     cairo_move_to (cr, (width-extents.width)/2, (extents.height+height)/2);
     cairo_show_text (cr, msg);

    cairo_restore(cr);
}

// Draw info in given area (in pixels)
void Video::drawInfo(cairo_t *cr, float left,  float top, float width, float height) const {
    cairo_save(cr);
    cairo_translate(cr,left,top);
    cairo_rectangle(cr,0.0,0.0,width,height);
    cairo_clip(cr);

    const float leftmargin=5;
    const float firstline=8;
    const float lineskip=15;
    float curline=firstline;

    cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

     std::ostringstream msg;

     curline+=lineskip*3;
     cairo_move_to (cr, leftmargin, curline);curline+=lineskip;
     msg << frame;
     cairo_set_font_size (cr, 40);
     cairo_show_text (cr, msg.str().c_str()); 

     cairo_set_font_size (cr, 10);

     for (unsigned int i=0;i<0;i++) {
	 cairo_move_to (cr, leftmargin, curline);curline+=lineskip;
	 msg.str("Line ");
	 msg << i;
	 cairo_show_text (cr, msg.str().c_str());
     }

     cairo_restore(cr);
}

// Draw in device coodinates
void Video::drawDevice(cairo_t *cr, float left, float top, float width, float height, Laser *laser)  {
    const std::vector<etherdream_point> &points = laser->getPoints();
    cairo_save(cr);
    cairo_translate(cr,left,top);
    cairo_rectangle(cr,0.0,0.0,width,height);
    cairo_clip(cr);

    if (height>titleHeight*4) {
	std::ostringstream msg;
	msg << "Laser " << laser->getUnit();
	drawText(cr,0,0,width,titleHeight,msg.str().c_str());
	cairo_translate(cr,0,titleHeight);
	height-=titleHeight;
    }
    
     // Add a border
     const float BORDER=2.0;
     cairo_translate(cr,BORDER,BORDER);
     width-=2*BORDER;
     height-=2*BORDER;

     // Translate to center
     cairo_translate(cr,width/2.0,height/2.0);
     float scale=std::min(width/(MAXVALUE*2.0),height/(MAXVALUE*2.0));
     cairo_scale(cr,scale,scale);
     float pixel=1.0/scale;

     // Draw overall bounds
     cairo_set_line_width(cr,1*pixel);
     cairo_set_source_rgb (cr,1.0,1.0,1.0);
     std::vector<Point> devBounds = laser->getTransform().mapToDevice(bounds);
     cairo_move_to(cr,devBounds.back().X(), devBounds.back().Y());
     for (unsigned int i=0;i<devBounds.size();i++)
	 cairo_line_to(cr,devBounds[i].X(),devBounds[i].Y());
     cairo_stroke(cr);

     // Draw bounding box
     cairo_set_line_width(cr,1*pixel);
     Color c=laser->getLabelColor();
     cairo_set_source_rgb (cr,c.red(),c.green(),c.blue());
     cairo_move_to(cr,-MAXVALUE,-MAXVALUE);
     cairo_line_to(cr,-MAXVALUE,MAXVALUE);
     cairo_line_to(cr,MAXVALUE,MAXVALUE);
     cairo_line_to(cr,MAXVALUE,-MAXVALUE);
     cairo_line_to(cr,-MAXVALUE,-MAXVALUE);
     cairo_stroke(cr);

     // Draw anchor points
     for (unsigned int i=0;i<4;i++) {
	 Point pt=laser->getTransform().getDevPoint(i);
	 //	 Color c=Color::getBasicColor(i);
	 //	 cairo_set_source_rgb (cr,c.red(), c.green(), c.blue());
	 cairo_move_to(cr,pt.X(),pt.Y());
	 cairo_arc(cr,pt.X(),pt.Y(),10*pixel,-i*M_PI/2,-i*M_PI/2+3*M_PI/2);
	 cairo_line_to(cr,pt.X(),pt.Y());
	 cairo_stroke(cr);
	 xrefs.refresh(cr,laser,i,true,pt);
     }

     // Draw points
     dbg("Video.drawDevice",2) << "Drawing " << points.size() << " points" << std::endl;
     Color maxColor=laser->getMaxColor();
     if (points.size()>1) {
	 cairo_set_line_width(cr,1*pixel);
	 etherdream_point lastpt = points[points.size()-1];
	 short minx=32767;
	 short maxx=-32768;
	 for (unsigned int i=0;i<points.size();i++) {
	     etherdream_point pt = points[i];
	     cairo_set_source_rgb (cr,std::min(maxColor.red(),pt.r/65535.0f),std::min(maxColor.green(),pt.g/65535.0f),std::min(maxColor.blue(),pt.b/65535.0f));
	     cairo_move_to(cr, lastpt.x,lastpt.y);
	     cairo_line_to(cr, pt.x, pt.y);
	     cairo_stroke(cr);
	     lastpt=pt;
	     minx=std::min(minx,pt.x);
	     maxx=std::max(maxx,pt.x);
	 }
	 dbg("Video.drawDevice",2) << "XRange: [" << minx << "," << maxx << "]" << std::endl;
     }
    cairo_restore(cr);
}

// Draw in world coodinates
void Video::drawWorld(cairo_t *cr, float left, float top, float width, float height, Lasers &lasers)  {
    cairo_save(cr);
    cairo_translate(cr,left,top);
    cairo_rectangle(cr,0.0,0.0,width,height);
    cairo_clip(cr);

    if (height>titleHeight*4) {
	drawText(cr,0,0,width,titleHeight,"World");
	cairo_translate(cr,0,titleHeight);
	height-=titleHeight;
    }

     // Add a border
     const float BORDER=2.0;
     cairo_translate(cr,BORDER,BORDER);
     width-=2*BORDER;
     height-=2*BORDER;

     dbg("Video.drawWorld",3) << "width=" << width << ", height=" << height << std::endl;

     assert(bounds.size()>=2);
     float minLeft=bounds[0].X();
     float maxRight=bounds[0].X();
     float minBottom=bounds[0].Y();
     float maxTop=bounds[0].Y();
     for (unsigned int i=1;i<bounds.size();i++) {
	 minLeft=std::min(minLeft, bounds[i].X());
	 maxRight=std::max(maxRight, bounds[i].Y());
	 minBottom=std::min(minBottom, bounds[i].Y());
	 maxTop=std::max(maxTop, bounds[i].X());
     }
     cairo_translate(cr,width/2.0,height/2.0);

     float scale=std::min((float)width/(maxRight-minLeft),(float)height/(maxTop-minBottom));
     cairo_scale(cr,scale,scale);
     float pixel=1.0/scale;
     dbg("Video.drawWorld",3) << "minLeft=" << minLeft << ", maxRight=" << maxRight << ", minBottom=" << minBottom << ", maxTop=" << maxTop << ", scale=" << scale << ", pixel=" << pixel << std::endl;
     cairo_translate(cr,-(minLeft+maxRight)/2,-(minBottom+maxTop)/2);

     // Draw overall bounds
     cairo_set_line_width(cr,1*pixel);
     cairo_set_source_rgb (cr,1.0,1.0,1.0);
     cairo_move_to(cr,bounds.back().X(), bounds.back().Y());
     for (unsigned int i=0;i<bounds.size();i++)
	 cairo_line_to(cr,bounds[i].X(),bounds[i].Y());
     cairo_stroke(cr);

     cairo_set_operator(cr,CAIRO_OPERATOR_ADD);   // Add colors

     for (unsigned int m=0;m<lasers.size();m++) {
	 Laser *laser=lasers.getLaser(m);
	 const std::vector<etherdream_point> &points=laser->getPoints();
	 const Transform &transform=laser->getTransform();

	 // Use laser-specific color
	 Color c=laser->getLabelColor();
	 cairo_set_source_rgb (cr,c.red(),c.green(),c.blue());

	 // Draw coverage area of laser
	 // Translate to center
	 etherdream_point tmp;
	 tmp.x=-MAXVALUE; tmp.y=-MAXVALUE;
	 Point worldTL=transform.mapToWorld(tmp);
	 tmp.x=MAXVALUE; tmp.y=-MAXVALUE;
	 Point worldTR=transform.mapToWorld(tmp);
	 tmp.x=-MAXVALUE; tmp.y=MAXVALUE;
	 Point worldBL=transform.mapToWorld(tmp);
	 tmp.x=MAXVALUE; tmp.y=MAXVALUE;
	 Point worldBR=transform.mapToWorld(tmp);
	 dbg("Video.drawWorld",3) << "TL=" << worldTL << ", TR=" << worldTR << ", BL=" << worldBL << ", BR=" << worldBR << std::endl;
	 cairo_set_line_width(cr,1*pixel);
	 cairo_move_to(cr,worldTL.X(),worldTL.Y());
	 cairo_line_to(cr,worldTR.X(),worldTR.Y());
	 cairo_line_to(cr,worldBR.X(),worldBR.Y());
	 cairo_line_to(cr,worldBL.X(),worldBL.Y());
	 cairo_line_to(cr,worldTL.X(),worldTL.Y());
	 cairo_stroke(cr);

	 // Draw anchor points
	 for (unsigned int i=0;i<4;i++) {
	     Point pt=laser->getTransform().getFloorPoint(i);
	     //Color c=Color::getBasicColor(i);
	     //	     cairo_set_source_rgb (cr,c.red(), c.green(), c.blue());
	     cairo_move_to(cr,pt.X(),pt.Y());
	     cairo_arc(cr,pt.X(),pt.Y(),10*pixel,-i*M_PI/2,-i*M_PI/2+3*M_PI/2);
	     cairo_line_to(cr,pt.X(),pt.Y());
	     cairo_stroke(cr);
	     xrefs.refresh(cr,laser,i,false,pt);
	 }

	 // Draw points
	 if (points.size() > 1) {
	     cairo_set_line_width(cr,1*pixel);
	     etherdream_point lastpt = points[points.size()-1];
	     Point lastwpt=transform.mapToWorld(lastpt);
	     for (unsigned int i=0;i<points.size();i++) {
		 etherdream_point pt = points[i];
		 Point wpt=transform.mapToWorld(pt);
		 dbg("Video.drawWorld",4) << "dev=[" <<  pt.x << "," << pt.y << "], world=" << wpt << std::endl;
		 if (pt.r > 0.0 || pt.g >0.0 || pt.b >0.0) {
		     cairo_move_to(cr, lastwpt.X(),lastwpt.Y());
		     cairo_line_to(cr, wpt.X(), wpt.Y());
		     cairo_stroke(cr);
		 }
		 lastwpt=wpt;
	     }
	 }
     }
    cairo_restore(cr);
}

// Draw given point set using device coords, and, in another frame, with device coordinates mapped back to world coords
void Video::update() {
    if (surface==NULL)
	return;
    lock();

    cairo_surface_flush(surface);
    float width=cairo_xlib_surface_get_width(surface);
    float height=cairo_xlib_surface_get_height(surface);
     cairo_t *cr = cairo_create(surface);

     // Erase surface
     cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
     cairo_paint(cr);

     // Draw info display
     const double rows[]={0.85*height,0.15*height};
     const double columns[]={0.5*width, 0.5*width };

     int nrow=(int)((lasers.size()+1)/2);
     int ncol=std::min(2,(int)lasers.size());
     int i=0;

     for (int row=0;row<nrow;row++) {
	 for (int col=0;col<ncol;col++) {
	     if (i>=(int)lasers.size())
		 break;
	     dbg("Video.update",2) << "Drawing laser " << i << " at row " << row << "/" << nrow << ", col " << col << "/" << ncol << std::endl;
	     drawDevice(cr, col*columns[0]/ncol, row*rows[0]/nrow, columns[0]/ncol, rows[0]/nrow,lasers.getLaser(i));
	     i++;
	 }
     }
     drawWorld(cr,columns[0],0.0f,columns[1],rows[0],lasers);
     drawInfo(cr,0.0f,rows[0],width,rows[1]);

     cairo_show_page(cr);
     cairo_destroy(cr);
     XFlush(dpy);
     unlock();
}

void Video::lock() {
    dbg("Video.lock",5) << "lock req" << std::endl;
    pthread_mutex_lock(&mutex);
    dbg("Video.lock",5) << "lock acquired" << std::endl;
}

void Video::unlock() {
    dbg("Video.unlock",5) << "unlock" << std::endl;
    pthread_mutex_unlock(&mutex);
}
