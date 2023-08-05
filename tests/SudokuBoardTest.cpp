#include "pch.h"

#include <gtest/gtest.h>

#include "../SudokuBoard.h"

TEST(TestSolveClassicBoard, solveBoardCorrect1) {
  Board initialBoard{
      {0, 5, 0, 2, 0, 0, 0, 4, 0}, {0, 0, 4, 5, 0, 0, 0, 0, 6},
      {6, 0, 0, 0, 0, 0, 0, 2, 0}, {4, 3, 7, 0, 0, 9, 0, 0, 0},
      {2, 6, 0, 7, 0, 0, 0, 5, 0}, {1, 0, 5, 4, 0, 6, 0, 0, 3},
      {0, 4, 0, 0, 0, 1, 0, 0, 0}, {0, 1, 2, 6, 7, 0, 0, 0, 0},
      {0, 0, 0, 0, 4, 2, 7, 1, 0},
  };

  Board solvedBoard{
      {9, 5, 1, 2, 6, 8, 3, 4, 7}, {3, 2, 4, 5, 1, 7, 8, 9, 6},
      {6, 7, 8, 9, 3, 4, 5, 2, 1}, {4, 3, 7, 1, 5, 9, 6, 8, 2},
      {2, 6, 9, 7, 8, 3, 1, 5, 4}, {1, 8, 5, 4, 2, 6, 9, 7, 3},
      {7, 4, 3, 8, 9, 1, 2, 6, 5}, {8, 1, 2, 6, 7, 5, 4, 3, 9},
      {5, 9, 6, 3, 4, 2, 7, 1, 8},
  };

  SudokuBoard sudokuBoard(initialBoard, Blocks());
  auto result = sudokuBoard.getCompletedBoard();
  EXPECT_EQ(solvedBoard, result);
}

TEST(TestSolveClassicBoard, solveBoardCorrect2) {
  Board initialBoard{
      {0, 0, 7, 0, 4, 0, 3, 5, 0}, {4, 0, 0, 0, 9, 0, 0, 0, 6},
      {0, 0, 1, 0, 0, 0, 0, 4, 0}, {0, 0, 0, 0, 0, 2, 0, 6, 1},
      {0, 0, 0, 9, 1, 0, 8, 0, 5}, {1, 8, 0, 0, 3, 6, 4, 0, 0},
      {8, 0, 4, 0, 0, 1, 0, 7, 0}, {0, 0, 0, 4, 0, 0, 0, 0, 3},
      {0, 2, 0, 0, 7, 5, 0, 0, 4},
  };

  Board solvedBoard{
      {2, 6, 7, 1, 4, 8, 3, 5, 9}, {4, 5, 8, 2, 9, 3, 7, 1, 6},
      {9, 3, 1, 6, 5, 7, 2, 4, 8}, {5, 4, 3, 7, 8, 2, 9, 6, 1},
      {6, 7, 2, 9, 1, 4, 8, 3, 5}, {1, 8, 9, 5, 3, 6, 4, 2, 7},
      {8, 9, 4, 3, 6, 1, 5, 7, 2}, {7, 1, 5, 4, 2, 9, 6, 8, 3},
      {3, 2, 6, 8, 7, 5, 1, 9, 4},
  };

  SudokuBoard sudokuBoard(initialBoard, Blocks());
  auto result = sudokuBoard.getCompletedBoard();
  EXPECT_EQ(solvedBoard, result);
}