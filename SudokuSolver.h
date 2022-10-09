#pragma once

#include <vector>

constexpr int kDimension = 9;

class SudokuSolver {
public:
  SudokuSolver(std::vector<std::vector<int>> board);
  bool solve();
  std::vector<std::vector<int>> getResults();
  void printResults();

private:
  bool isPresentInCol(int col, int num);
  bool isPresentInRow(int row, int num);
  bool isPresentInBox(int boxStartRow, int boxStartCol, int num);
  bool findEmptyPlace(int& row, int& col);
  bool isValidPlace(int row, int col, int num);

  std::vector<std::vector<int>> _board;
};