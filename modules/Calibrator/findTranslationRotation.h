#pragma once

#include "opencv2/core.hpp"

// Find a 3x3 matrix that maps src points to dst points using only rotation and translation
cv::Mat findTranslationRotation( const std::vector<cv::Point2f> &src,  const std::vector<cv::Point2f> &dst);


