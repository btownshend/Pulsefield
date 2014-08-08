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
    CPoints applyMovements(std::string attrname, float attrValue, const CPoints &pts) const;
    CPoints applyDashes(std::string attrname, float attrValue,const CPoints &pts) const;
    CPoints applyMusic(std::string attrname, float attrValue,const CPoints &pts) const;
    CPoints applyStraighten(std::string attrname, float attrValue,const CPoints &pts) const;
    CPoints applyDoubler(std::string attrname, float attrValue,const CPoints &pts) const;
    static float getTotalLen(const CPoints &pts);
public:
    Attributes() { ; }
    void set(std::string name, const Attribute &a) {	attrs[name]=a;  }
    const Attribute &get(std::string name) const { return attrs.at(name); }
    Attributes filter(float minval) const;   // Keep only attributes >= minval
    Attributes keepStrongest() const;  // Reduce to the single strongest attribute
    float getMaxVal() const;  // Get max attribute value
    void erase(std::string name) { attrs.erase(name); }
    void clear() { attrs.clear(); }
    friend std::ostream &operator<<(std::ostream &s, const Attributes &attributes);
    unsigned int size() const { return attrs.size(); }
    // Apply all attributes to a vector of points
    CPoints apply(const CPoints &pts) const;
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


