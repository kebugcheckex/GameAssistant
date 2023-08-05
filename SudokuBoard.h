#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Defs.h"

constexpr std::string_view kBlocksSymbols{"+-*=@#$%&"};
constexpr std::string_view kHorizontalLine = "-------------------------\n";

class SudokuBoard {
 public:
  /*
   * Construct a Sudoku board with some initial numbers. For irregular mode,
   * `blocks` is a vector of unordered sets where each row's index is the block
   * ID and elements in each row is the 1D index representation of the coordinates
   */
  SudokuBoard(const Board& initialBoard, const Blocks& blocks);

  /*
   * Completed board is the board with all cells filled with correct numbers
   */
  Board getCompletedBoard();

  /*
   * Solved board is the board that only contains new numbers filled while
   * cells with existing numbers are set to 0
   */
  Board getSolvedBoard();

  // Utility functions
  // TODO change the order of the two params
  static void printBoard(const std::string& title, const Board& board);
  static void printBlocks(const Blocks& blocks, const std::string& title = "");

  /*
   * Convert a coordinate (row, col) into the 1D index representation
   */
  static int convertCoordinateToIndex(int row, int col);

  /*
   * Convert a 1D index to its coordinate form (row, col), opposite to
   * convertCoordinateToIndex
   */
  static std::pair<int, int> convertIndexToCoordinate(int index);

  static std::unordered_map<int, int> createBlocksMap(const Blocks& blocks);
 private:
  bool isPresentInCol(int col, int num);
  bool isPresentInRow(int row, int num);
  bool isPresentInBlock(int row, int col, int num);
  bool findEmptyPlace(int& row, int& col);
  bool isValidPlace(int row, int col, int num);
  bool solve();

  Board board_;
  Board initialBoard_;
  Blocks blocks_;

  // maps a cell's coordinate (in 1d index form) to its block ID
  std::unordered_map<int, int> blocksMap_;
};