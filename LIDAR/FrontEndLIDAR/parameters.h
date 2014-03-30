/*
 * parameters.h
 *
 *  Created on: Mar 29, 2014
 *      Author: bst
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

static const int MINBGSEP=100;	// max distance to consider a scan point part of a nearby background
static const int MINRANGE=100;	// minimum distance from LIDAR; ranges less than this are ignored
static const float MAXTGTSEP=100; 	// Max separation between points that are still the same target (unless there is a gap between)
static const float INITLEGDIAM=200;	// Initial diameter of legs
static const float MAXLEGDIAM=220;	// Maximum diameter of legs
static const float MAXCLASSSIZE=300;	// Maximum size of a single class in meters before it needs to be split
static const float MINTARGET=100;	// Minimum unshadowed target size (otherwise is noise)
#endif  /* PARAMETERS_H_ */
