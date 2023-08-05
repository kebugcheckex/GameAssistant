#pragma once

#include <tuple>
#include <unordered_set>
#include <opencv2/core.hpp>

#define DOUBLE_FOR_LOOP       \
  for (int i = 0; i < 9; i++) \
    for (int j = 0; j < 9; j++)
    

enum GameMode { CLASSIC, IRREGULAR, ICE_BREAKER };

typedef std::tuple<int, int, int> Ice;

typedef std::tuple<int, int, int> IceBreakerCell;

typedef std::vector<std::vector<int>> Board;
typedef std::vector<std::unordered_set<int>> Blocks;
typedef std::vector<cv::Point> Contour;

constexpr double EPS = 1e-6;
constexpr int kDimension = 9;
