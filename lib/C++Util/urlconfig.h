#pragma once

#define MAXURLS 100

class URLConfig {
    std::string filename;
    int nurl;
    char *idents[MAXURLS];
    char *hosts[MAXURLS];
    int ports[MAXURLS];
    int getIndex(const char *ident) const;
 public: 
    // Load  config file
    URLConfig(const char *filename);
    ~URLConfig();
    // Get host for ident or null if not found
    const char *getHost(const char *ident) const;
    // Get port for ident or -1 if not found
    int getPort(const char *ident) const;
    // Get filename
    std::string getFilename() const { return filename; }
};
