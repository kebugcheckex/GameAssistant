#pragma once

#include <Windows.h>

#include <utility>
#include <vector>

#include "GameWindow.h"
#include "SudokuBoard.h"
#include "SudokuData.h"
#include "SudokuRecognizer.h"

/*
 * Some useful command line options
 * Classic: --game-mode classic --fill-order row --stop-after 3 --play-interval
 * 1000 Irregular: --game-mode irregular --fill-order col --stop-after 4
 * --play-interval 1000 Icebreaker: --game-mode icebreaker --debug
 */

namespace game_assistant {
enum FillOrder { ROW, COLUMN, BLOCK };

class Player {
 public:
  Player(std::shared_ptr<GameWindow> gameWindow,
         std::shared_ptr<sudoku::SudokuRecognizer> recognizer,
         std::shared_ptr<sudoku::SudokuBoard> sudokuBoard,
         sudoku::GameMode gameMode);

  void play();

 private:
  void playNormalBoard(FillOrder fillOrder);
  void playIceBreaker();

  void fillAt(int row, int col, char value /* numerical value */);

  int gridSize_;
  cv::Rect boardRect_;
  sudoku::GameMode gameMode_;
  std::shared_ptr<GameWindow> gameWindow_;
  std::shared_ptr<sudoku::SudokuRecognizer> recognizer_;
  std::shared_ptr<sudoku::SudokuBoard> sudokuBoard_;
};

}  // namespace game_assistant
