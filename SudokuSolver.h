#pragma once

#include <vector>
#include <string>

constexpr int kDimension = 9;

class SudokuSolver {
public:
  SudokuSolver(std::vector<std::vector<int>> board);
  bool solve();
  std::vector<std::vector<int>> getResults();
  // Solved board means only values that do not exist in the original board are included
  // Cells that initially contain values are set to zero.
  std::vector<std::vector<int>> getSolvedBoard();
  static void printBoard(const std::string& title, const std::vector<std::vector<int>>& board);

private:
  bool isPresentInCol(int col, int num);
  bool isPresentInRow(int row, int num);
  bool isPresentInBox(int boxStartRow, int boxStartCol, int num);
  bool findEmptyPlace(int& row, int& col);
  bool isValidPlace(int row, int col, int num);

  std::vector<std::vector<int>> board_;
  std::vector<std::vector<int>> originalBoard_;
};