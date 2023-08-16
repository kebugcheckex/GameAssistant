#include "pch.h"

#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include "../RecognizerUtils.h"

namespace game_assistant {
namespace utils {
TEST(TestCalculateAngle, rightAngle) {
  cv::Point vertex(0, 0), side1(2, 0), side2(0, 3);
  auto angle = calculateCosineAngle(vertex, side1, side2);
  EXPECT_LT(angle, 1e-5);
}

TEST(TestCalculateAngle, angle45Degree) {
  cv::Point vertex(0, 0), side1(3, 0), side2(0, 3);
  auto angle = calculateCosineAngle(vertex, side1, side2);
  EXPECT_LT(angle - std::sqrt(2) / 2, 1e-5);
}

}  // namespace utils
}  // namespace game_assistant
