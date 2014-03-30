/*
 * people.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef PEOPLE_H_
#define PEOPLE_H_

#include <vector>
#include "person.h"

class People {
    std::vector<Person> persons;
public:
    People();
    mxArray *convertToMX() const;
};

#endif  /* PEOPLE_H_ */
