#pragma once
#include "point.h"

#ifdef DRAWING
std::vector<Point> findTargets( const std::vector<Point> background, void *drawing=NULL);
#else
std::vector<Point> findTargets( const std::vector<Point> background);
#endif

