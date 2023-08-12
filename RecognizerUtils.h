#pragma once

#include <opencv2/core.hpp>

#include "SudokuData.h"

constexpr std::string_view kCvWindowName{"Game Assistant"};

namespace game_assistant {
namespace utils {

typedef std::vector<cv::Point> Contour;

enum DebugColors {
  Red,
  Green,
  Blue,
  Yellow,
  Cyan,
  Magenta,
  Purple,
  Orange,
  Teal
};

extern const std::vector<cv::Scalar> kDebugColors;

// Calculate the cosine value of the angble between two vectors
double calculateCosineAngle(cv::Point vertex, cv::Point side1, cv::Point side2);

/*
 * From a vector of contours find rectangles (not rotated) and optionally
 * sort the results by their areas in decending order
 */
std::vector<cv::Rect> findRectangles(const std::vector<Contour>& contours,
                                     bool sortByAreaDesc = false);

/*
 * Check if the given contour is a rectangle. A rectangle must have four and
 * only four vertices. The four vertices must be ordered either clockwise or
 * counter-clockwise direction.
 * TODO rotated not implemented yet
 */
bool isRectangle(const Contour& contour, bool rotated = false);

cv::Rect convertContourToRect(const Contour& contour);

void printCvRect(const cv::Rect& rect);

int getRandomInt(int min, int max);

void sortContourByArea(std::vector<Contour>& contours,
                              bool descending = false);

void showImage(const cv::Mat& image, const std::string& title = "");

void sortContourByArea(std::vector<Contour>& contours, bool descending);

}  // namespace utils
}  // namespace game_assistant