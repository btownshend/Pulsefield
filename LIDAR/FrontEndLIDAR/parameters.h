/*
 * parameters.h
 *
 *  Created on: Mar 29, 2014
 *      Author: bst
 */

#pragma once

#include <math.h>

// Distances in mm unless otherwise noted

static const char PROTOVERSION[]="2.1";	 // Protocol version

static const int MAXGRIDPTS=1000;	// Maximum grid points during likelihood calculation
static const float UNITSPERM=1000.0;		// Internal units per meter

// ******** Sensor
static const float SENSORSIGMA=8;	// Sensor statistical error 
static const double RANDOMPTPROB=1e-30;	// Probability any given scan point is random noise

// ******** Background
static const int MINBGSEP=100;	// max distance to consider a scan point part of a nearby background
static const float MINBGFREQ=0.05;	// Minimum frequency of a background to call it such unilaterally
static const int BGINITFRAMES=50*5;		// Background intiialization for this many frames
static const int UPDATETC=50*60;		// Background update freq (after initial averaging)
static const int UPDATETCCLOSER=150*60;		// Background update freq (after initial averaging) for new background is closer than dominant
static const int UPDATETCFARTHER=20*60;		// Background update freq (after initial averaging) for new background is farther than dominant
static const unsigned int MINRANGE=100;	// minimum distance from LIDAR; ranges less than this are ignored
extern unsigned int MAXRANGE;	// maximum distance from LIDAR; ranges greater than this are ignored
static const float ADJSCANBGWEIGHT=0.2;	// scaling of background probability when applying an adjacent scan's background to a point
static const float INTERPSCANBGWEIGHT=0.2;	// scaling of background probability when interpolating between adjacent scan backgrounds

// ***** Assignment
static const float MINFORCELIKE=-10;  // Minimum likelihood to force assigning a class to the only target that is possible (otherwise a track is formed)

// ***** Tracking
static const float INITIALPOSITIONVAR=100*100;  // Variance of initial position estimates
static const float MAXPOSITIONVAR=1000*1000;  // Never let the position variance go above this during predictions
static const float INITIALHIDDENVAR=300*300;  // Variance of initial position estimates when leg is not initially visible
static const float MAXLEGSPEED=4000;	// Maximum speed of a leg in mm/s
static const float MAXLEGSEP=400;	// Maximum separation of leg centers
static const float VELUPDATETC=10;	// Velocity update time constant in frames
static const float VELDAMPING=0.95;	// Damping (multiplicative factor) for legvelocity when not visible
static const float DRIFTVAR=100*100;	// Additional variance of position estimates per step when they are estimated 
static const float MINLIKEFORUPDATES=-30;	  // Minimum likelihood of a target to use the current observations to update it

// ******** Leg statistics 
static const float INITLEGDIAM=200;	// Initial diameter of legs
static const float LEGDIAMSIGMA=50;		// Sigma for leg diameter
static const float MEANLEGSEP=300; 	// Mean leg separation
static const float LEGSEPSIGMA=100;		// Sigma for leg separation
static const float FACINGSEM=20.0*M_PI/180; 	// SEM for facing direction 
static const float LEGSTATSTC=100; 	// Time constant for updating  estimate of leg diameter, separation, etc.
static const float LEFTNESSTC=500;	// time constant for updating leftness

// ******** Deleting tracks
static const int INVISIBLEFORTOOLONG=50;	// Number of frames of invisible before deleting
static const int AGETHRESHOLD=20;	// Age before considered reliable    
static const float MINVISIBILITY=0.9;	// Minimum visibility to maintain new tracks (before ageThreshold reached)

// ******* Entry Statistics
static const float ENTRYRATE=1.0; 	// Entries per minute
static const int MINCREATEHITS=5;	// Need this number of scan targets to create a new track

// ******** Grouping
static const float GROUPDIST=500;		// Distance to form a group
static const float UNGROUPDIST=1000;	// Distance to break a group

// ******** Channels
static const int NCHANNELS=16;
