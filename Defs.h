#pragma once

#include <tuple>

enum GameMode { CLASSIC, IRREGULAR, ICE_BREAKER };

typedef std::tuple<int, int, int> Ice;

typedef std::tuple<int, int, int> IceBreakerCell;

typedef std::vector<std::vector<int>> Board;
