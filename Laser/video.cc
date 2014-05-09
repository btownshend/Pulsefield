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
    XSelectInput(world->dpy, w, StructureNotifyMask | ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    XMapWindow(world->dpy, w);

    world->surface = cairo_xlib_surface_create(world->dpy, w, DefaultVisual(world->dpy, 0), 800, 400);

    while (1) {
	XEvent e;
	XNextEvent(world->dpy, &e);
	dbg("Video.runDisplay",8) << "Got event " << e.type << std::endl;

	switch (e.type) {
	case ButtonPress:
	    std::cout << "Button Pressed:  " << e.xbutton.x << ", " << e.xbutton.y << std::endl;
	    break;
	case ButtonRelease:
	    std::cout << "Button Released:  " << e.xbutton.x << ", " << e.xbutton.y << std::endl;
	    break;
	case MotionNotify:
	    std::cout << "Motion:  " << e.xmotion.x << ", " << e.xmotion.y << std::endl;
	    break;
	case ConfigureNotify:
	    cairo_xlib_surface_set_size(world->surface,e.xconfigure.width, e.xconfigure.height);
	case MapNotify:
	case Expose:
	    //world->draw();
	    break;
	}
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

// Draw in device coodinates
void Video::drawDevice(cairo_t *cr, float left, float top, float width, float height, const std::vector<etherdream_point> &points, int unit) const {
    cairo_save(cr);
    cairo_translate(cr,left,top);
    cairo_rectangle(cr,0.0,0.0,width,height);
    cairo_clip(cr);

    if (height>titleHeight*4) {
	std::ostringstream msg;
	msg << "Laser " << unit;
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
     float pixel=MAXVALUE*1.0/std::min(width,height);

     // Draw bounding box
     cairo_set_line_width(cr,1*pixel);
     cairo_set_source_rgb (cr,1.0,1.0,1.0);
     cairo_move_to(cr,-MAXVALUE,-MAXVALUE);
     cairo_line_to(cr,-MAXVALUE,MAXVALUE);
     cairo_line_to(cr,MAXVALUE,MAXVALUE);
     cairo_line_to(cr,MAXVALUE,-MAXVALUE);
     cairo_line_to(cr,-MAXVALUE,-MAXVALUE);
     cairo_stroke(cr);

     // Draw points
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

    cairo_restore(cr);
}

// Draw in world coodinates
void Video::drawWorld(cairo_t *cr, float left, float top, float width, float height, const std::vector<etherdream_point> &points, const Transform &transform) const {
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
     dbg("Video.drawWorld",3) << "width=" << width << ", height=" << height << std::endl;

     float minLeft=std::min(std::min(worldTL.X(), worldTR.X()),std::min(worldBL.X(), worldBR.X()));
     float maxRight=std::max(std::max(worldTL.X(), worldTR.X()),std::max(worldBL.X(), worldBR.X()));
     float minBottom=std::min(std::min(worldTL.Y(), worldTR.Y()),std::min(worldBL.Y(), worldBR.Y()));
     float maxTop=std::max(std::max(worldTL.Y(), worldTR.Y()),std::max(worldBL.Y(), worldBR.Y()));
     cairo_translate(cr,width/2.0,height/2.0);

     float scale=std::min((float)width/(maxRight-minLeft),(float)height/(maxTop-minBottom));
     cairo_scale(cr,scale,scale);
     float pixel=1.0/scale;
     dbg("Video.drawWorld",3) << "scale=" << scale << ", pixel=" << pixel << std::endl;
     cairo_translate(cr,-(minLeft+maxRight)/2,-(minBottom+maxTop)/2);

     // Draw bounding box
     cairo_set_line_width(cr,1*pixel);
     cairo_set_source_rgb (cr,1.0,1.0,1.0);
     cairo_move_to(cr,worldTL.X(),worldTL.Y());
     cairo_line_to(cr,worldTR.X(),worldTR.Y());
     cairo_line_to(cr,worldBR.X(),worldBR.Y());
     cairo_line_to(cr,worldBL.X(),worldBL.Y());
     cairo_line_to(cr,worldTL.X(),worldTL.Y());
     cairo_stroke(cr);

     // Draw points
     cairo_set_line_width(cr,1*pixel);
     etherdream_point lastpt = points[points.size()-1];
     Point lastwpt=transform.mapToWorld(lastpt);
     for (unsigned int i=0;i<points.size();i++) {
	 etherdream_point pt = points[i];
	 Point wpt=transform.mapToWorld(pt);
	 dbg("Video.drawWorld",4) << "dev=[" <<  pt.x << "," << pt.y << "], world=" << wpt << std::endl;
	 cairo_set_source_rgb (cr,pt.r/65535.0,pt.g/65535.0,pt.b/65535.0);
	 cairo_move_to(cr, lastwpt.X(),lastwpt.Y());
	 cairo_line_to(cr, wpt.X(), wpt.Y());
	 cairo_stroke(cr);
	 lastwpt=wpt;
     }

    cairo_restore(cr);
}

// Draw given point set using device coords, and, in another frame, with device coordinates mapped back to world coords
void Video::update(const std::vector<etherdream_point> &points, const Transform &transform) {
    if (surface==NULL)
	return;
    cairo_surface_flush(surface);
    float width=cairo_xlib_surface_get_width(surface);
    float height=cairo_xlib_surface_get_height(surface);
     cairo_t *cr = cairo_create(surface);

     // Erase surface
     cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
     cairo_paint(cr);

     // Draw info display
     const float rows[]={0.85,0.15};
     const float columns[]={0.25, 0.25, 0.5 };

     drawDevice(cr, 0.0f, 0.0f, width*columns[0], rows[0]*height/2,points,1);
     drawDevice(cr, columns[0]*width, 0.0f, width*columns[1], rows[0]*height/2,points,2);
     drawDevice(cr, 0.0f, rows[0]/2*height, width*columns[0], rows[0]*height/2,points,3);
     drawDevice(cr, columns[0]*width, rows[0]/2*height, width*columns[1], rows[0]*height/2,points,4);

     drawWorld(cr,width*(columns[0]+columns[1]),0.0f,width*columns[2],rows[0]*height,points,transform);

     drawInfo(cr,0.0f,rows[0]*height,width,rows[1]*height);

     cairo_show_page(cr);
     cairo_destroy(cr);
     XFlush(dpy);
}

void Video::update(const std::vector<etherdream_point> &points) {
    update(points,Transform());
}
