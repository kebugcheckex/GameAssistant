/*
  Shamelessly copied from
  https://www.tutorialspoint.com/Sudoku-Solving-algorithms with some
  modification
*/

#include "pch.h"
#include "SudokuSolver.h"

SudokuSolver::SudokuSolver(std::vector<std::vector<int>> board) {
  _board = board;
}

bool SudokuSolver::isPresentInCol(int col, int num) {
  for (int row = 0; row < kDimension; row++) {
    if (_board[row][col] == num) { return true; }
  }
  return false;
}

bool SudokuSolver::isPresentInRow(int row, int num) {
  for (int col = 0; col < kDimension; col++) {
    if (_board[row][col] == num) { return true; }
  }
  return false;
}

bool SudokuSolver::isPresentInBox(int boxStartRow, int boxStartCol, int num) {
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (_board[row + boxStartRow][col + boxStartCol] == num) { return true; }
    }
  }
  return false;
}

bool SudokuSolver::findEmptyPlace(int& row, int& col) {  // get empty location and update row and column
  for (row = 0; row < kDimension; row++) {
    for (col = 0; col < kDimension; col++) {
      if (_board[row][col] == 0) {
        return true;
      }
    }
  }
  return false;
}

bool SudokuSolver::isValidPlace(int row, int col, int num) {
  // when item not found in col, row and current 3x3 box
  auto valid = !isPresentInRow(row, num) && !isPresentInCol(col, num) &&
    !isPresentInBox(row - row % 3, col - col % 3, num);
  return valid;
}

bool SudokuSolver::solve() {
  int row, col;
  if (!findEmptyPlace(row, col)) {
    std::cout << "All places are filled\n";
    return true;
  }
  for (int num = 1; num <= 9; num++) {     //valid numbers are 1 - 9
    if (isValidPlace(row, col, num)) {    //check validation, if yes, put the number in the grid
      _board[row][col] = num;
      if (solve())     // recursively go for other rooms in the grid
        return true;
      _board[row][col] = 0;    //turn to unassigned space when conditions are not satisfied
    }
  }
  return false;
}

std::vector<std::vector<int>> SudokuSolver::getResults() {
  solve();
  return _board;
}

void SudokuSolver::printResults() {
  constexpr std::string_view kHorizontalLine = "-------------------------\n";
  std::cout << kHorizontalLine;
  for (int i = 0; i < 9; i++) {
    std::cout << "| ";
    for (int j = 0; j < 9; j++) {
      std::cout << _board[i][j] << " ";
      if (j % 3 == 2) {
        std::cout << "| ";
      }
    }
    std::cout << "\n";
    if (i % 3 == 2) {
      std::cout << kHorizontalLine;
    }
  }
  std::cout << "\n";
}