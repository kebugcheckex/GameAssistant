#pragma once

#include <vector>
#include <utility>

#include <Windows.h>

class Player {
public:
  Player(const RECT& monitorRect, const RECT& boardRect);

  // board should only contain values that needs to be filled. Existing numbers should be
  // set to zero.
  // boardRect is the absolute coordinates of the board (relative to the whole desktop/monitor)
 void play(const std::vector<std::vector<int>>& board);
  static LRESULT CALLBACK handleHotKey(int nCode, WPARAM wParam, LPARAM lParam);

 private:
  // Click at absolute screen location (x, y)
 void clickAt(int x, int y);
 void pressKey(char ch);  // TODO maybe extend this to other virtual key codes
 void fillAt(int row, int col, char value /* numerical value */);
 
 int screenWidth_, screenHeight_, gridSize_;
 RECT boardRect_;

 static bool shouldStop_;
};