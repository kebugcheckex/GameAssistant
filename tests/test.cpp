#include "pch.h"

#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include "../RecognizerUtils.h"

TEST(TestCalculateAngle, rightAngle) {
  cv::Point vertex(0, 0), side1(2, 0), side2(0, 3);
  auto angle = RecognizerUtils::calculateAngle(vertex, side1, side2);
  EXPECT_LT(abs(angle - 90.f), 1e-5);
}

// TODO unit test not working, need to figure out how to configure VS