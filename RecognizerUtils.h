#pragma once

#include <opencv2/core.hpp>
#include "Defs.h"

class RecognizerUtils {
 public:
  /*
   * Calculate the angle of two vectors
   */
  static double calculateAngle(cv::Point vertex, cv::Point side1,
                               cv::Point side2);

  /*
   * From a vector of contours find rectangles (not rotated) and optionally
   * sort the results by their areas in decending order
   */
  static std::vector<cv::Rect> findRectangles(
      const std::vector<Contour>& contours,
      bool sortByAreaDesc = false);

  static void printCvRect(const cv::Rect& rect);
};
