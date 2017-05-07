#include <map>

class Wrapper {
    // Handle wrap-arounds of any integer
    std::string name;
    unsigned int maxval;
    std::map<unsigned int,unsigned int> lastValue;
    std::map<unsigned int,unsigned int> nWraps;
public:
    Wrapper(std::string _name, unsigned int _maxval) {
	name=_name; maxval=_maxval; 
	dbg("Wrapper.Wrapper",1) << "Created new Wrapper for " << name << " with maxval " << maxval << std::endl;
    }
    // Wrap given value (must be called with non-decreasing values of val for a given key
    unsigned int wrap(unsigned int key, unsigned int val) {
	if (val < lastValue[key]) {
	    dbg("Wrapper.lookup",1) << "Wrapping of " << name << " for key  " << key << ": went from " << lastValue[key]<< " to " << val << std::endl;
	    nWraps[key]++;
	}
	lastValue[key]=val;
	return nWraps[key];
    }
};
