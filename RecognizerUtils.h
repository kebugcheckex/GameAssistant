#pragma once

#include <opencv2/core.hpp>
#include "Defs.h"

class RecognizerUtils {
 public:
  /*
   * Calculate the cosine value of the angble between two vectors
   */
  static double calculateCosineAngle(cv::Point vertex, cv::Point side1,
                               cv::Point side2);

  /*
   * From a vector of contours find rectangles (not rotated) and optionally
   * sort the results by their areas in decending order
   */
  static std::vector<cv::Rect> findRectangles(
      const std::vector<Contour>& contours,
      bool sortByAreaDesc = false);

  /*
   * Check if the given contour is a rectangle. A rectangle must have four and
   * only four vertices. The four vertices must be ordered either clockwise or
   * counter-clockwise direction.
   * TODO rotated not implemented yet
   */
  static bool isRectangle(const Contour& contour, bool rotated = false);

  static cv::Rect convertContourToRect(const Contour& contour);

  static void printCvRect(const cv::Rect& rect);

  static int getRandomInt(int min, int max);

  static void sortContourByArea(std::vector<Contour>& contours,
                                bool descending = false);
};
