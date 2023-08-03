#include "pch.h"

#include "RecognizerUtils.h"

#include <opencv2/imgproc.hpp>

#include "Defs.h"

// static
double RecognizerUtils::calculateAngle(cv::Point vertex, cv::Point side1,
                                       cv::Point side2) {
  double dx1 = side1.x - vertex.x;
  double dy1 = side1.y - vertex.y;
  double dx2 = side2.x - vertex.x;
  double dy2 = side2.y - vertex.y;
  return (dx1 * dx2 + dy1 * dy2) /
         sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

// static
std::vector<cv::Rect> RecognizerUtils::findRectangles(
    const std::vector<Contour>& contours, bool sortByAreaDesc) {
  std::vector<cv::Rect> rectangles;
  bool isRectangle = true;
  for (const auto& contour : contours) {
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, cv::arcLength(contour, true) * 0.02,
                     true);
    if (approx.size() != 4) {
      continue;
    }
    for (int i = 0; i < 4; i++) {
      auto angle = calculateAngle(contour[(i + 1) % 4], contour[i],
                                  contour[(i + 2) % 4]);
      if (fabs(angle - 90. > EPS)) {
        isRectangle = false;
        break;
      }
    }
    if (isRectangle) {
      rectangles.emplace_back(approx[0], approx[2]);
    }
  }

  if (sortByAreaDesc) {
    std::sort(rectangles.begin(), rectangles.end(), [](cv::Rect a, cv::Rect b) {
      return (a.width * a.height) < (b.width * b.height);
    });
  }
  return rectangles;
}

// static
void RecognizerUtils::printCvRect(const cv::Rect& rect) {
  std::cout << fmt::format("Rectangle ({}, {}) -> ({}, {})\n", rect.x, rect.y,
                           rect.x + rect.width, rect.y + rect.height);
}