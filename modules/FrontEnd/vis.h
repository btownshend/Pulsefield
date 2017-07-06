/*
 * vis.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#pragma once

#include "sickio.h"

class Vis {
    const SickIO *sick;
public:
    Vis();
    void update(const SickIO *sick);

#ifdef MATLAB
    // Convert to an mxArray
    mxArray *convertToMX(int frame) const;
#endif
    const SickIO *getSick() const { return sick; }
};
