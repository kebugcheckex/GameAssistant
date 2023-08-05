/*
  Shamelessly copied from
  https://www.tutorialspoint.com/Sudoku-Solving-algorithms with some
  modification
*/

#include "pch.h"

#include "SudokuBoard.h"

SudokuBoard::SudokuBoard(const Board& initialBoard, const Blocks& blocks)
    : board_(initialBoard), initialBoard_(initialBoard), blocks_(blocks) {
  printBoard("Initial Board", initialBoard);

  if (blocks_.size() == 0) {
    blocks_.resize(kDimension);
    DOUBLE_FOR_LOOP {
      int coord = i * 9 + j;
      int blockId = (i / 3) * 3 + j / 3;
      blocks_[blockId].insert(coord);
    }
  }
  blocksMap_ = createBlocksMap(blocks_);
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

bool SudokuBoard::isPresentInBox(int boxStartRow, int boxStartCol, int num) {
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (board_[row + boxStartRow][col + boxStartCol] == num) {
        return true;
      }
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
      board_[row][col] =
          0;  // turn to unassigned space when conditions are not satisfied
    }
  }
  return false;
}

Board SudokuBoard::getCompletedBoard() {
  solve();
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

/* static */
void SudokuBoard::printBoard(const std::string& title, const Board& board) {
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