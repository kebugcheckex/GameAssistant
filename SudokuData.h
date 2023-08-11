#pragma once

#include <opencv2/core.hpp>
#include <tuple>
#include <unordered_set>

#define DOUBLE_FOR_LOOP       \
  for (int i = 0; i < 9; i++) \
    for (int j = 0; j < 9; j++)

namespace game_assistant {
namespace sudoku {
enum GameMode { CLASSIC, IRREGULAR, ICE_BREAKER };

typedef std::tuple<int, int, int> Ice;

typedef std::tuple<int, int, int> IceBreakerCell;

typedef std::vector<std::vector<int>> Board;
typedef std::vector<std::unordered_set<int>> Blocks;

constexpr int kDimension = 9;

}  // namespace sudoku
}  // namespace game_assistant
