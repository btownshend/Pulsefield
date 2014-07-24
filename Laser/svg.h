#include <string>
#include <map>
#include "nanosvg.h"

class SVG  {
    NSVGimage *image;
    std::string filename;
    float bounds[4];   // minx,miny,maxx,maxy
    SVG(NSVGimage *image);
 public:
    static std::shared_ptr<SVG> load(std::string filename);
    void addToDrawing(Drawing &d,Point origin, float scale, float rotateDeg, Color c) const;
    float width() const { return image->width; }
    float height() const { return image->height; }
};


class SVGs {
    static std::map<std::string,std::shared_ptr<SVG> > svgs;
 public:
    static std::shared_ptr<SVG> get(std::string filename) {
	if (svgs.count(filename)==0)
	    svgs[filename]=SVG::load(filename);

	return svgs[filename];
    }
};

