#pragma once

#include <ostream>
#include <string>
#include <vector>

class Destinations;

class Destination {
    friend class Destinations;
    std::string host;
    int port;
    int failcount;
 public:
    Destination(std::string _host, int _port) {
	host=_host;
	port=_port;
	failcount=0;
    }
    friend std::ostream& operator<<(std::ostream &s, const Destination &d);
};

class Destinations {
    std::vector<Destination> dests;
 public:
    Destinations();
    ~Destinations();
    void add(const char *host, int port);
    void remove(const char *host, int port);
    void remove(int i);
    void removeAll();
    int size() const { return dests.size(); }
    const char *getHost(int i) const { return dests[i].host.c_str(); }
    int getPort(int i) const { return dests[i].port; }
    void setFailed(int i) {dests[i].failcount++;}
    void setSucceeded(int i) {dests[i].failcount=0;}
    int getFailCount(int i) const { return dests[i].failcount; }
};
