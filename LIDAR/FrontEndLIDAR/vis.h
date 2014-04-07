/*
 * vis.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef VIS_H_
#define VIS_H_
#include "sickio.h"

class Vis {
    const SickIO *sick;
public:
    Vis();
    void update(const SickIO *sick);

    // Convert to an mxArray
    mxArray *convertToMX() const;

    const SickIO *getSick() const { return sick; }
};

#endif  /* VIS_H_ */
