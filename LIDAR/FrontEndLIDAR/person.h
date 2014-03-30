/*
 * person.h
 *
 *  Created on: Mar 25, 2014
 *      Author: bst
 */

#ifndef PERSON_H_
#define PERSON_H_

#include <mat.h>

class Person {
    // Overall 
    int id;
    float position[2];
    float velocity[2];
    int age;
    int consecutiveInvisibleCount;
    int totalVisibleCount;

    // Leg positions, etc
    float legs[2][2];
    float prevlegs[2][2];
    float legvelocity[2][2];
    float legclasses[2];
    float posvar[2];
    float legdiam;
    float leftness;
public:
    Person();
    void addToMX(mxArray *people, int index) const;
};

#endif  /* PERSON_H_ */
