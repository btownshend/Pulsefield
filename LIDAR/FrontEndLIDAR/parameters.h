/*
 * parameters.h
 *
 *  Created on: Mar 29, 2014
 *      Author: bst
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

// Distances in mm unless otherwise noted

// ******** Background
static const int MINBGSEP=100;	// max distance to consider a scan point part of a nearby background
static const float MINBGFREQ=0.05;	// Minimum frequency of a background to call it such unilaterally
static const int UPDATETC=50*60;		// Background update freq (after initial averaging)
static const unsigned int MINRANGE=100;	// minimum distance from LIDAR; ranges less than this are ignored
static const unsigned int MAXRANGE=6000;	// maximum distance from LIDAR; ranges greater than this are ignored
static const float ADJSCANBGWEIGHT=0.2;	// scaling of background probability when applying an adjacent scan's background to a point
static const float INTERPSCANBGWEIGHT=0.2;	// scaling of background probability when interpolating between adjacent scan backgrounds

// ***** Classifier
static const float MAXTGTSEP=100; 	// Max separation between points that are still the same target (unless there is a gap between)
// Making MAXTGTSEP too small results in splitting one target into two and then creates a new extraneous person for one of them 
// Making it too big can merge 2 legs together resulting in the other leg being placed in a shadow, this would occur if one leg is behind the other, partially occluded.   If it 
// side-by-side the total class size is >MAXCLASSSIZE, it will be split before that happens
static const float INITLEGDIAM=200;	// Initial diameter of legs
static const float MAXLEGDIAM=220;	// Maximum diameter of legs
static const float MAXCLASSSIZE=300;	// Maximum size of a single class in meters before it needs to be split
static const float MINTARGET=100;	// Minimum unshadowed target size (otherwise is noise)

// ***** Assignment
static const float MINFORCELIKE=-10;  // Minimum likelihood to force assigning a class to the only target that is possible (otherwise a track is formed)

// ***** Tracking
static const float INITIALPOSITIONVAR=100*100;  // Variance of initial position estimates
static const float INITIALHIDDENVAR=300*300;  // Variance of initial position estimates when leg is not initially visible
static const float MAXLEGSPEED=4000;	// Maximum speed of a leg in mm/s
static const float MAXLEGSEP=400;	// Maximum separation of leg centers
static const float MAXMOVEMENT=1000;	// Maximum amount of movement in mm per update
static const float VELUPDATETC=10;	// Velocity update time constant in frames
static const float VELDAMPING=0.95;	// Damping (multiplicative factor) for legvelocity when not visible
static const float LEGDIAMTC=100; 	// Time constant for updating diameter estimate of leg
static const float DRIFTVAR=50*50;	// Additional variance of position estimates per step when they are estimated 
static const float NEWTRACKEQUIVDIST1=3000;	// log-likelihood of adding a new track for a single class is equivalent to matching an existing class at this distance
static const float NEWTRACKEQUIVDIST2=2000;	// log-likelihood of adding a new track for a class pair is equivalent to matching an existing class at this distance
static const float LEFTNESSTC=500;	// time constant for updating leftness
static const float HIDDENPENALTY=3;	// loglike penalty when using a shadowed position for a leg
static const float YOUNGPENALTY=5;	// loglike penalty applied to "young" tracks (<AGETHRESHOLD)
static const float HIDDENLEGSCALING=0.8;  // Scale diameter of leg by this much when looking for a place it could be shadowed

// ******** Deleting tracks
static const int INVISIBLEFORTOOLONG=50;	// Number of frames of invisible before deleting
static const int AGETHRESHOLD=20;	// Age before considered reliable    
static const float MINVISIBILITY=0.9;	// Minimum visibility to maintain new tracks (before ageThreshold reached)

#endif  /* PARAMETERS_H_ */
