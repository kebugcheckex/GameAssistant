#pragma once

#include <Windows.h>

#include <utility>
#include <vector>

#include "Defs.h"
#include "GameWindow.h"
#include "SudokuRecognizer.h"
#include "SudokuSolver.h"

class Player {
 public:
  Player(std::shared_ptr<GameWindow> gameWindow,
         std::shared_ptr<SudokuRecognizer> recognizer,
         std::shared_ptr<SudokuSolver> solver, GameMode gameMode);

  void play();

 private:
  void playClassic();
  void playIrregular();
  void playIceBreaker();
  void playIceBreakerNew();
  
  std::pair<int, int> findNextIceBreakerLocation(const Board& iceBoard,
                                  const Board& weightBoard);

  // returns false if no more ice present
  bool updateIceBoard(Board& iceBoard, const std::pair<int, int>& location);
  
  void updateWeightBoard(Board& weightBoard, const Board& iceBoard,
                         const Board& solvedBoard);

  // Click at absolute screen location (x, y)
  void clickAt(int x, int y);
  void pressKey(char ch);  // TODO maybe extend this to other virtual key codes
  void fillAt(int row, int col, char value /* numerical value */);

  int screenWidth_, screenHeight_, gridSize_;
  RECT boardRect_;
  GameMode gameMode_;
  std::shared_ptr<GameWindow> gameWindow_;
  std::shared_ptr<SudokuRecognizer> recognizer_;
  std::shared_ptr<SudokuSolver> solver_;
};