#pragma once

#include <vector>
#include <string>

#include "Defs.h"

constexpr int kDimension = 9;

class SudokuSolver {
public:
  SudokuSolver(Board board);
  bool solve();
  Board getResults();
  // Solved board means only values that do not exist in the original board are included
  // Cells that initially contain values are set to zero.
  Board getSolvedBoard();
  static void printBoard(const std::string& title, const Board& board);

private:
  bool isPresentInCol(int col, int num);
  bool isPresentInRow(int row, int num);
  bool isPresentInBox(int boxStartRow, int boxStartCol, int num);
  bool findEmptyPlace(int& row, int& col);
  bool isValidPlace(int row, int col, int num);

  Board board_;
  Board originalBoard_;
};