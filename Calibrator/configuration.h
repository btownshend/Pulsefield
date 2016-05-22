#pragma once
#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

class Configuration {
    std::string filename;
    ptree root;
 public:
    Configuration(std::string fname) {
	filename=fname;
    }
    void load();
    void save() const;
    ptree &pt() { return root; }
    const ptree &pt() const { return root; }
};
