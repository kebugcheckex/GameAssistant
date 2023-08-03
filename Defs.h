#pragma once

#include <tuple>
#include <opencv2/core.hpp>

enum GameMode { CLASSIC, IRREGULAR, ICE_BREAKER };

typedef std::tuple<int, int, int> Ice;

typedef std::tuple<int, int, int> IceBreakerCell;

typedef std::vector<std::vector<int>> Board;

typedef std::vector<cv::Point> Contour;

constexpr double EPS = 1e-6;
