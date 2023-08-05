#include "pch.h"
#include "Player.h"

#include <fmt/core.h>
#include <glog/logging.h>

#include <queue>

static bool validateFillOrder(const char* flagName, const std::string& value) {
  if (value != "row" && value != "col" && value != "row|col") {
    std::cerr << "Invalid value for --" << flagName << " " << value << "\n";
    return false;
  }
  return true;
}

DEFINE_string(fill_order, "row|col", "Fill by rows or columns");
DEFINE_validator(fill_order, &validateFillOrder);
DEFINE_int32(play_interval, 3000,
             "Time interval between automatic play actions");
DEFINE_int32(
    stop_after, 9,
    "Stop playing after finishing certain number of rows/columns");

Player::Player(std::shared_ptr<GameWindow> gameWindow,
               std::shared_ptr<SudokuRecognizer> recognizer,
               std::shared_ptr<SudokuBoard> solver, GameMode gameMode)
    : gameWindow_(gameWindow),
      recognizer_(recognizer),
      solver_(solver),
      gameMode_(gameMode) {
  auto monitorRect = gameWindow_->getMonitorRect();
  screenWidth_ = monitorRect.right - monitorRect.left;
  screenHeight_ = monitorRect.bottom - monitorRect.top;

  auto windowRect = gameWindow_->getWindowRect();
  boardRect_ = recognizer_->getBoardRect();
  boardRect_.left += windowRect.left;
  boardRect_.top += windowRect.top;
  boardRect_.right += windowRect.left;
  boardRect_.bottom += windowRect.top;
  DLOG(INFO) << fmt::format("Board Rect: ({}, {}) -> ({}, {})\n",
                            boardRect_.left, boardRect_.top, boardRect_.right,
                            boardRect_.bottom);
  gridSize_ = (boardRect_.right - boardRect_.left) / 9;
}

void Player::play() {
  switch (gameMode_) {
    case GameMode::CLASSIC:
    case GameMode::IRREGULAR:
      playNormalBoard();
      break;
    case GameMode::ICE_BREAKER:
      playIceBreaker();
      break;
    default:
      LOG(FATAL) << "Unknown game mode " << gameMode_;
  }
}

// TODO irregular board need a separate logic for fill N blocks
void Player::playNormalBoard() {
  auto board = solver_->getSolvedBoard();
  SudokuBoard::printBoard("Solved board", board);

  if (FLAGS_fill_order != "row" && FLAGS_fill_order != "col") {
    LOG(ERROR) << "No fill order specified. Will not auto play\n";
    return;
  }

  LOG(INFO) << "Auto-play started";
  // Need to click in the window first to make sure it gets focus
  clickAt(boardRect_.left, boardRect_.top - 50);
  Sleep(3000);  // there is an animation before the screen settles

  for (int i = 0; i < FLAGS_stop_after; i++) {
    for (int j = 0; j < 9; j++) {
      if (FLAGS_fill_order == "row") {
        if (board[i][j] == 0) {
          continue;
        }
        fillAt(i, j, (char)board[i][j]);
      } else if (FLAGS_fill_order == "col") {
        if (board[j][i] == 0) {
          continue;
        }
        fillAt(j, i, (char)board[j][i]);
      }
    }
  }
  LOG(INFO) << "Auto-play completed";
}

void Player::playIceBreaker() {
  auto solvedBoard = solver_->getSolvedBoard();
  SudokuBoard::printBoard("Completed board", solver_->getCompletedBoard());
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
  solvedBoard = solver_->getSolvedBoard();  
  clickAt(boardRect_.left, boardRect_.top - 50);
  Sleep(3000);  // there is an animation before the screen settles

  for (const auto& step : steps) {
    auto [row, col] = step;
    LOG(INFO) << fmt::format("Place {} at ({}, {})\n", solvedBoard[row][col], row + 1, col + 1);
    fillAt(row, col, (char)solvedBoard[row][col]);
  }
  LOG(INFO) << "Auto-play completed.\n";
}

void Player::clickAt(int x, int y) {
  INPUT inputs[3] = {};
  inputs[0].type = INPUT_MOUSE;
  inputs[0].mi.dx = (int)((float)x / screenWidth_ * 65535.f);
  inputs[0].mi.dy = (int)((float)y / screenHeight_ * 65535.f);
  inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
  inputs[1].type = INPUT_MOUSE;
  inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  inputs[2].type = INPUT_MOUSE;
  inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
  DLOG(INFO) << fmt::format("Clicking at ({}, {})\n", x, y);
  SendInput(3, inputs, sizeof(INPUT));
}

void Player::pressKey(char ch) {
  INPUT inputs[2] = {};
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = ch;
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = ch;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(2, inputs, sizeof(INPUT));
}

void Player::fillAt(int row, int col, char value) {
  int x = boardRect_.left + col * gridSize_ + gridSize_ / 2;
  int y = boardRect_.top + row * gridSize_ + gridSize_ / 2;
  clickAt(x, y);
  Sleep(1000);
  pressKey('0' + value);
  Sleep(FLAGS_play_interval);
}
