#pragma once

#include <Windows.h>

#include <utility>
#include <vector>

#include "Defs.h"
#include "GameWindow.h"
#include "SudokuRecognizer.h"
#include "SudokuBoard.h"

class Player {
 public:
  Player(std::shared_ptr<GameWindow> gameWindow,
         std::shared_ptr<SudokuRecognizer> recognizer,
         std::shared_ptr<SudokuBoard> solver, GameMode gameMode);

  void play();

 private:
  void playNormalBoard();
  void playIceBreaker();

  // TODO move the following functions to GameWindow class
  // Click at absolute screen location (x, y)
  void clickAt(int x, int y);
  void pressKey(char ch);  // TODO maybe extend this to other virtual key codes
  void fillAt(int row, int col, char value /* numerical value */);

  int screenWidth_, screenHeight_, gridSize_;
  RECT boardRect_;
  GameMode gameMode_;
  std::shared_ptr<GameWindow> gameWindow_;
  std::shared_ptr<SudokuRecognizer> recognizer_;
  std::shared_ptr<SudokuBoard> solver_;
};