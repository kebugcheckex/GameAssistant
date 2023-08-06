#pragma once

#include <Windows.h>

#include <utility>
#include <vector>

#include "Defs.h"
#include "GameWindow.h"
#include "SudokuRecognizer.h"
#include "SudokuBoard.h"

enum FillOrder {ROW, COLUMN, BLOCK};

class Player {
 public:
  Player(std::shared_ptr<GameWindow> gameWindow,
         std::shared_ptr<SudokuRecognizer> recognizer,
         std::shared_ptr<SudokuBoard> sudokuBoard, GameMode gameMode);

  void play();

 private:
  void playNormalBoard(FillOrder fillOrder);
  void playIceBreaker();
  
  void fillAt(int row, int col, char value /* numerical value */);

  int gridSize_;
  cv::Rect boardRect_;
  GameMode gameMode_;
  std::shared_ptr<GameWindow> gameWindow_;
  std::shared_ptr<SudokuRecognizer> recognizer_;
  std::shared_ptr<SudokuBoard> sudokuBoard_;
};