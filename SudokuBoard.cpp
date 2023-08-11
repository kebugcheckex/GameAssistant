/*
  Shamelessly copied from
  https://www.tutorialspoint.com/Sudoku-Solving-algorithms with some
  modification
*/

#include "pch.h"

#include "SudokuBoard.h"

#include <fmt/core.h>

namespace game_assistant {
namespace sudoku {
SudokuBoard::SudokuBoard(const Board& initialBoard, const Blocks& blocks)
    : board_(initialBoard), initialBoard_(initialBoard), blocks_(blocks) {
  if (blocks_.size() == 0) {
    blocks_.resize(kDimension);
    DOUBLE_FOR_LOOP {
      int coord = i * 9 + j;
      int blockId = (i / 3) * 3 + j / 3;
      blocks_[blockId].insert(coord);
    }
  }
  blocksMap_ = createBlocksMap(blocks_);
  SudokuBoard::printBoard(initialBoard_, "Initial Board");
}

bool SudokuBoard::isPresentInCol(int col, int num) {
  for (int row = 0; row < kDimension; row++) {
    if (board_[row][col] == num) {
      return true;
    }
  }
  return false;
}

bool SudokuBoard::isPresentInRow(int row, int num) {
  for (int col = 0; col < kDimension; col++) {
    if (board_[row][col] == num) {
      return true;
    }
  }
  return false;
}

bool SudokuBoard::isPresentInBlock(int row, int col, int num) {
  int blockId = blocksMap_[convertCoordinateToIndex(row, col)];
  for (const auto& cell : blocks_[blockId]) {
    auto [row, col] = convertIndexToCoordinate(cell);
    if (board_[row][col] == num) {
      return true;
    }
  }
  return false;
}

bool SudokuBoard::findEmptyPlace(int& row, int& col) {
  for (row = 0; row < kDimension; row++) {
    for (col = 0; col < kDimension; col++) {
      if (board_[row][col] == 0) {
        return true;
      }
    }
  }
  return false;
}

bool SudokuBoard::isValidPlace(int row, int col, int num) {
  return !isPresentInRow(row, num) && !isPresentInCol(col, num) &&
         !isPresentInBlock(row, col, num);
}

bool SudokuBoard::solve() {
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
      board_[row][col] = 0;
    }
  }
  return false;
}

Board SudokuBoard::getCompletedBoard() {
  if (!solve()) {
    LOG(ERROR) << "failed to solve the board";
    printBoard(initialBoard_, "Initial Board");
  }
  return board_;
}

Board SudokuBoard::getSolvedBoard() {
  solve();
  Board solvedBoard = board_;
  for (int i = 0; i < kDimension; i++) {
    for (int j = 0; j < kDimension; j++) {
      if (initialBoard_[i][j] != 0) {
        solvedBoard[i][j] = 0;
      }
    }
  }
  return solvedBoard;
}

Blocks SudokuBoard::getBlocks() { return blocks_; }

// static
void SudokuBoard::printBoard(const Board& board, const std::string& title) {
  // TODO fix a few issues here and write unit tests
  std::cout << "====================\n";
  std::cout << title << "\n";
  std::cout << "====================\n";
  if (board.empty()) {
    std::cout << "Board is empty!\n";
    return;
  }
  std::vector<int> columnWidths(9, 1);

  for (int col = 0; col < 9; col++) {
    for (int row = 0; row < 9; row++) {
      if (board[row][col] == -1) {
        columnWidths[col] = 2;
        break;
      }
    }
  }
  std::cout << kHorizontalLine;
  for (int i = 0; i < 9; i++) {
    std::cout << "| ";
    for (int j = 0; j < 9; j++) {
      std::cout << fmt::format("{0:{1}} ", board[i][j], columnWidths[j]);
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

// static
void SudokuBoard::printBlocks(const Blocks& blocks, const std::string& title) {
  std::cout << "====================\n";
  std::cout << "Blocks: " << title << "\n";
  std::cout << "====================\n";
  auto blocksMap = createBlocksMap(blocks);

  std::cout << kHorizontalLine;
  for (int i = 0; i < kDimension; i++) {
    std::cout << "| ";
    for (int j = 0; j < kDimension; j++) {
      int blockId = blocksMap[convertCoordinateToIndex(i, j)];
      std::cout << kBlocksSymbols.at(blockId) << " ";
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

// static
int SudokuBoard::convertCoordinateToIndex(int row, int col) {
  return row * kDimension + col;
}

// static
std::pair<int, int> SudokuBoard::convertIndexToCoordinate(int index) {
  return {index / kDimension, index % kDimension};
}

// static
std::unordered_map<int, int> SudokuBoard::createBlocksMap(
    const Blocks& blocks) {
  std::unordered_map<int, int> blocksMap;
  for (std::size_t i = 0; i < kDimension; i++) {
    const auto& block = blocks[i];
    for (auto index : block) {
      blocksMap.insert({index, i});
    }
  }
  return blocksMap;
}

}  // namespace sudoku
}  // namespace game_assistant
