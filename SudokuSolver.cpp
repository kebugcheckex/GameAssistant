/*
  Shamelessly copied from
  https://www.tutorialspoint.com/Sudoku-Solving-algorithms with some
  modification
*/

#include "pch.h"
#include <fmt/core.h>
#include "SudokuSolver.h"

SudokuSolver::SudokuSolver(Board board) {
  board_ = board;
  originalBoard_ = board;
  printBoard("Original Board", originalBoard_);
}

bool SudokuSolver::isPresentInCol(int col, int num) {
  for (int row = 0; row < kDimension; row++) {
    if (board_[row][col] == num) {
      return true;
    }
  }
  return false;
}

bool SudokuSolver::isPresentInRow(int row, int num) {
  for (int col = 0; col < kDimension; col++) {
    if (board_[row][col] == num) {
      return true;
    }
  }
  return false;
}

bool SudokuSolver::isPresentInBox(int boxStartRow, int boxStartCol, int num) {
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (board_[row + boxStartRow][col + boxStartCol] == num) {
        return true;
      }
    }
  }
  return false;
}

bool SudokuSolver::findEmptyPlace(int& row, int& col) {
  for (row = 0; row < kDimension; row++) {
    for (col = 0; col < kDimension; col++) {
      if (board_[row][col] == 0) {
        return true;
      }
    }
  }
  return false;
}

bool SudokuSolver::isValidPlace(int row, int col, int num) {
  auto valid = !isPresentInRow(row, num) && !isPresentInCol(col, num) &&
               !isPresentInBox(row - row % 3, col - col % 3, num);
  return valid;
}

bool SudokuSolver::solve() {
  int row, col;
  if (!findEmptyPlace(row, col)) {
    return true;
  }
  for (int num = 1; num <= 9; num++) {
    if (isValidPlace(row, col, num)) {
      board_[row][col] = num;
      if (solve()) {
        return true;
      }
      board_[row][col] =
          0;  // turn to unassigned space when conditions are not satisfied
    }
  }
  return false;
}

Board SudokuSolver::getCompletedBoard() {
  solve();
  return board_;
}

Board SudokuSolver::getSolvedBoard() {
  solve();
  Board solvedBoard = board_;
  for (int i = 0; i < kDimension; i++) {
    for (int j = 0; j < kDimension; j++) {
      if (originalBoard_[i][j] != 0) {
        solvedBoard[i][j] = 0;
      }
    }
  }
  return solvedBoard;
}

/* static */
void SudokuSolver::printBoard(const std::string& title, const Board& board) {
    // TODO fix a few issues here and write unit tests
  constexpr std::string_view kHorizontalLine = "-------------------------\n";
  std::cout << "====================\n";
  std::cout << title << "\n";
  std::cout << "====================\n";
  std::cout << kHorizontalLine;
  std::vector<int> columnWidths(9, 1);

  for (int col = 0; col < 9; col++) {
    for (int row = 0; row < 9; row++) {
      if (board[row][col] == -1) {
        columnWidths[col] = 2;
        break;
      }
    }
  }
  for (int i = 0; i < 9; i++) {
    std::cout << "| ";
    for (int j = 0; j < 9; j++) {
      std::cout << fmt::format("{0: {1}} ", board[i][j], columnWidths[j]);
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