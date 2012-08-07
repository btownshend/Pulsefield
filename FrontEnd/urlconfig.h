#ifndef URLCONFIG_H
#define URLCONFIG_H

#define MAXURLS 100
#define MAXLEN 50

class URLConfig {
    char *filename;
    int nurl;
    char idents[MAXURLS][MAXLEN];
    char hosts[MAXURLS][MAXLEN];
    int ports[];
    int getIndex(const char *ident);
 public:
    // Load  config file
    URLConfig(const char *filename);
    // Get host for ident or null if not found
    const char *getHost(const char *ident);
    // Get port for ident or -1 if not found
    int getPort(const char *ident);
    // Get filename
    const char *getFilename() { return filename; }
};
#endif
