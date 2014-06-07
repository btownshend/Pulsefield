#pragma once
#include <map>

#include "cpoint.h"
#include "Simplex.hpp"
#include "touchosc.h"

// An attribute of a connection or person
class Attribute {
    float value;
    float time;
 public:
    Attribute() {;}
    Attribute(float _value, float _time) { value=_value; time=_time;}
    float getValue() const { return value; }
    float getTime() const { return time; }
    friend std::ostream &operator<<(std::ostream &s, const Attribute &c);
};


class Attributes {
    std::map<std::string,Attribute> attrs;
    std::vector<CPoint> applyMovements(std::string attrname, float attrValue, const std::vector<CPoint> &pts) const;
    std::vector<CPoint> applyDashes(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const;
    std::vector<CPoint> applyMusic(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const;
    std::vector<CPoint> applyStraighten(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const;
    std::vector<CPoint> applyDoubler(std::string attrname, float attrValue,const std::vector<CPoint> &pts) const;
    static float getTotalLen(const std::vector<CPoint> &pts);
public:
    Attributes() { ; }
    void set(std::string name, const Attribute &a) {	attrs[name]=a;  }
    const Attribute &get(std::string name) const { return attrs.at(name); }
    void erase(std::string name) { attrs.erase(name); }
    void clear() { attrs.clear(); }
    friend std::ostream &operator<<(std::ostream &s, const Attributes &attributes);
    unsigned int size() const { return attrs.size(); }
    // Apply all attributes to a vector of points
    std::vector<CPoint> apply(std::vector<CPoint> pts) const;
    std::vector<std::string > getAttributeNames() const {
	std::vector<std::string > result;
	for (std::map<std::string ,Attribute>::const_iterator a=attrs.begin(); a!=attrs.end();a++) {
	    result.push_back(a->first);
	}
	std::sort(result.begin(),result.end());
	return result;
    }
    bool isSet(std::string attr) const { return attrs.count(attr) > 0; }
};

