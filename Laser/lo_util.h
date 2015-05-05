#pragma once
#include <cstdlib>
#include <lo/lo.h>

// Get remote URL taking care to free memory allocated by lo_
inline std::string loutil_address_get_url(lo_address remote)  {
    char *url=lo_address_get_url(remote);
    std::string result(url);
    free(url);
    return result;
}
