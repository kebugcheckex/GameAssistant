#pragma once

#include <string>
#include <vector>

#include "Defs.h"

constexpr int kDimension = 9;

class SudokuBoard {
 public:
  SudokuBoard(Board board);

  /*
   * Completed board is the board with all cells filled with correct numbers
   */
  Board getCompletedBoard();

  /*
   * Solved board is the board that only contains new numbers filled while
   * cells with existing numbers are set to 0
   */
  Board getSolvedBoard();


  static void printBoard(const std::string& title, const Board& board);

 private:
  bool isPresentInCol(int col, int num);
  bool isPresentInRow(int row, int num);
  bool isPresentInBox(int boxStartRow, int boxStartCol, int num);
  bool findEmptyPlace(int& row, int& col);
  bool isValidPlace(int row, int col, int num);
  bool solve();

  Board board_;
  Board originalBoard_;
};