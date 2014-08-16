#include <memory>
#include "drawing.h"

#define NANOSVG_IMPLEMENTATION
#include "svg.h"

std::map<std::string,std::shared_ptr<SVG> > SVGs::svgs;

SVG::SVG(NSVGimage *image) {
    this->image=image;
    for (int i=0;i<4;i++)
	bounds[i]=0;
    for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
	bounds[0]=std::min(bounds[0],shape->bounds[0]);
	bounds[1]=std::min(bounds[1],shape->bounds[1]);
	bounds[2]=std::max(bounds[2],shape->bounds[2]);
	bounds[3]=std::max(bounds[3],shape->bounds[3]);
    }
    dbg("SVG",1) << "Initialized SVG with bounds=" << bounds[0] << ", "  <<  bounds[1] << ", " <<  bounds[2] << ", " <<  bounds[3] << std::endl;
}

std::shared_ptr<SVG> SVG::load(std::string filename) {
    NSVGimage *image = nsvgParseFromFile(filename.c_str(), "px", 96.0f);
    if (image == NULL)  {
	std::cerr << "Could not open SVG image: " << filename << std::endl;
	return nullptr;
    }
    dbg("SVG.load",1) << "Loaded SVG from " << filename << std::endl;
    return std::shared_ptr<SVG>(new SVG(image));
}

void SVG::addToDrawing(Drawing &d,Point origin, float scale, float rotateDeg, Color c) const {
    if (image==NULL)
	return;
    int nshapes=0;
    int npaths=0;
    scale/=std::max(image->width,image->height);
    for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
	for (	NSVGpath* path = shape->paths; path != NULL; path = path->next) {
	    std::vector<Point> pts;
	    for (int i=0;i<path->npts*2;i+=2) {
		Point p(path->pts[i],path->pts[i+1]);
		p=p-Point(width()/2,height()/2);	// Shift to put origin in center of bounding box
		p=p.flipX(); // Flip x-axis
		p=p.rotateDegrees(rotateDeg);  // Rotate around center
		pts.push_back(p*scale+origin); // Scale and translate to desired location
	    }
	    d.drawPath(pts,c);
	    npaths++;
	}
	nshapes++;
    }
    dbg("SVG.addToDrawing",1) << "Added SVG at " << origin << " with " << nshapes << " shapes containing a total of " << npaths << " paths" << std::endl;
}
