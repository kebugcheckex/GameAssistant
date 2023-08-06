#include "pch.h"

#include "Player.h"

#include <fmt/core.h>
#include <glog/logging.h>

#include <queue>

std::unordered_map<std::string, FillOrder> const FillOrderOptions = {
    {"row", FillOrder::ROW},
    {"col", FillOrder::COLUMN},
    {"block", FillOrder::BLOCK},
};

// TODO maybe create a template to unify this one and validateGameMode in main.cpp?
static bool validateFillOrder(const char* flagName, const std::string& value) {
  if (!value.empty() && FillOrderOptions.find(value) == FillOrderOptions.end()) {
    LOG(ERROR) << fmt::format("Invalid value for option --{} {}", flagName,
                              value);
    return false;
  }
  return true;
}

DEFINE_string(fill_order, "", "Fill by row | col | block");
DEFINE_validator(fill_order, &validateFillOrder);
DEFINE_int32(play_interval, 3000,
             "Time interval between automatic play actions");
DEFINE_int32(stop_after, 9,
             "Stop playing after finishing certain number of rows/columns");

Player::Player(std::shared_ptr<GameWindow> gameWindow,
               std::shared_ptr<SudokuRecognizer> recognizer,
               std::shared_ptr<SudokuBoard> sudokuBoard, GameMode gameMode)
    : gameWindow_(gameWindow),
      recognizer_(recognizer),
      sudokuBoard_(sudokuBoard),
      gameMode_(gameMode) {
  boardRect_ = recognizer_->getBoardRect();
  gridSize_ = boardRect_.width / 9;
}

void Player::play() {
  if (gameMode_ != GameMode::ICE_BREAKER && FLAGS_fill_order.empty()) {
    LOG(INFO) << "--fill-order not set, will not auto-play";
    return;
  }

  switch (gameMode_) {
    case GameMode::CLASSIC:
    case GameMode::IRREGULAR:
      playNormalBoard(FillOrderOptions.at(FLAGS_fill_order));
      break;
    case GameMode::ICE_BREAKER:
      playIceBreaker();
      break;
    default:
      LOG(FATAL) << "Unknown game mode " << gameMode_;
  }
}

// TODO irregular board need a separate logic for fill N blocks
void Player::playNormalBoard(FillOrder fillOrder) {
  auto solvedBoard = sudokuBoard_->getSolvedBoard();

  LOG(INFO) << "Auto-play started";
  // Need to click in the window first to make sure it gets focus
  gameWindow_->clickAt(boardRect_.x - 10, boardRect_.y - 50);
  Sleep(3000);  // there is an animation before the screen settles

  if (fillOrder == FillOrder::BLOCK) {
    auto blocks = sudokuBoard_->getBlocks();
    for (int i = 0; i < FLAGS_stop_after; i++) {
      for (const int index : blocks[i]) {
        auto [row, col] = SudokuBoard::convertIndexToCoordinate(index);
        if (solvedBoard[row][col] == 0) {
          continue;
        }
        fillAt(row, col, (char)solvedBoard[row][col]);
      }
    }
  } else {
    for (int i = 0; i < FLAGS_stop_after; i++) {
      for (int j = 0; j < 9; j++) {
        if (fillOrder == FillOrder::ROW) {
          if (solvedBoard[i][j] == 0) {
            continue;
          }
          fillAt(i, j, (char)solvedBoard[i][j]);
        } else if (fillOrder == FillOrder::COLUMN) {
          if (solvedBoard[j][i] == 0) {
            continue;
          }
          fillAt(j, i, (char)solvedBoard[j][i]);
        } else {
          LOG(FATAL) << "Unknown fill order " << fillOrder;
        }
      }
    }
  }
  LOG(INFO) << "Auto-play completed";
}

void Player::playIceBreaker() {
  auto solvedBoard = sudokuBoard_->getSolvedBoard();
  SudokuBoard::printBoard("Completed board", sudokuBoard_->getCompletedBoard());
  auto iceBoard = recognizer_->getIceBoard();

  std::vector<std::pair<int, int>> steps;
  while (true) {
    Board weightBoard(9, std::vector<int>(9, 0));
    // For each ice cell, add the ice weight to all its row and column cells,
    // excluding cells with existing numbers or the other ice cells
    for (int row = 0; row < 9; row++) {
      for (int col = 0; col < 9; col++) {
        if (iceBoard[row][col] > 0) {
          for (int i = 0; i < 9; i++) {
            if (iceBoard[row][i] > 0 || solvedBoard[row][i] == 0) {
              continue;
            }
            weightBoard[row][i] += iceBoard[row][col];
          }
          for (int i = 0; i < 9; i++) {
            if (iceBoard[i][col] > 0 || solvedBoard[i][col] == 0) {
              continue;
            }
            weightBoard[i][col] += iceBoard[row][col];
          }
        }
      }
    }

    int maxWeight = std::numeric_limits<int>::min();
    int maxRow = -1, maxCol = -1;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        auto weight = weightBoard[i][j];
        if (weight > 0 && weight > maxWeight) {
          maxWeight = weight;
          maxRow = i;
          maxCol = j;
        }
      }
    }
    // make sure the location is not on an ice cell
    DCHECK_LE(iceBoard[maxRow][maxCol], 0);
    steps.push_back({maxRow, maxCol});

    // once a number is placed, remove it from solved board
    solvedBoard[maxRow][maxCol] = 0;

    for (int i = 0; i < 9; i++) {
      if (iceBoard[maxRow][i] > 0) {
        iceBoard[maxRow][i]--;
      }
      if (iceBoard[i][maxCol] > 0) {
        iceBoard[i][maxCol]--;
      }
    }

    bool foundIce = false;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        if (iceBoard[i][j] > 0) {
          foundIce = true;
          break;
        }
      }
      if (foundIce) {
        break;
      }
    }

    if (!foundIce) {
      break;  // no more ice cells
    }
  }
  // get a fresh copy as the previous copy has been modified
  solvedBoard = sudokuBoard_->getSolvedBoard();
  std::cout << "Auto-play started\n";
  // Need to click in the window first to make sure it gets focus
  gameWindow_->clickAt(boardRect_.x - 10, boardRect_.y - 50);
  Sleep(3000);  // there is an animation before the screen settles
  for (const auto& step : steps) {
    auto [row, col] = step;
    LOG(INFO) << fmt::format("Place {} at ({}, {})\n", solvedBoard[row][col],
                             row + 1, col + 1);
    fillAt(row, col, (char)solvedBoard[row][col]);
  }
  std::cout << "Auto-play completed.\n";
}

void Player::fillAt(int row, int col, char value) {
  int x = boardRect_.x + col * gridSize_ + gridSize_ / 2;
  int y = boardRect_.y + row * gridSize_ + gridSize_ / 2;
  gameWindow_->clickAt(x, y);
  Sleep(1000);
  gameWindow_->pressKey('0' + value);
  Sleep(FLAGS_play_interval);
}
