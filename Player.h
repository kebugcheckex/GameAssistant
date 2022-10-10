#pragma once

#include <vector>
#include <utility>

#include <Windows.h>

class Player {
public:
  Player(const RECT& monitorRect);
  // board should only contain values that needs to be filled. Existing numbers should be
  // set to zero.
  // boardRect is the absolute coordinates of the board (relative to the whole desktop/monitor)
 void play(const RECT& boardRect,
                  const std::vector<std::vector<int>>& board);

private:
  // Click at absolute screen location (x, y)
 void clickAt(int x, int y);
 void pressKey(char ch);  // TODO maybe extend this to other virtual key codes

int screenWidth_, screenHeight_;
};