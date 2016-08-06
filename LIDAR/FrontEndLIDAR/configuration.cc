#include <iostream>
#include "configuration.h"
#include "dbg.h"

#include <boost/property_tree/json_parser.hpp>

void Configuration::load() {
    dbg("Configuration.load",1) << "Loading configuration from " << filename << std::endl;
    try {
	read_json(filename,root);
    } catch (...) {
	std::cerr << "Unable to open settings file " << filename << std::endl;
    }
}

void Configuration::save() const {
    dbg("Configuration.save",1) << "Saving configuration to " << filename << std::endl;
    write_json(filename,root);
}

#ifdef TESTING

int main() {
    Configuration *s = Configuration::instance();

    s->load();
    int x=s->pt().get("a.x",0);
    std::cout << "x=" << x << std::endl;
    x=x+1;
    s->pt().put("a.x",x);
    s->save();
}

#endif
