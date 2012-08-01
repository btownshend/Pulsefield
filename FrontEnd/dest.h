class Destinations {
    int ndest;
    int maxdest;
    char **hosts;
    int *ports;
 public:
    Destinations();
    ~Destinations();
    void add(const char *host, int port);
    void remove(const char *host, int port);
    void removeAll();
    int count() const { return ndest; }
    const char *getHost(int i) const { return hosts[i]; }
    int getPort(int i) const { return ports[i]; }
};
