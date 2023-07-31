#include "pch.h"
#include "Player.h"

#include <fmt/core.h>
#include <glog/logging.h>

#include <queue>

#include "SudokuSolver.h"

static bool validateFillOrder(const char* flagName, const std::string& value) {
  if (value != "row" && value != "col") {
    std::cerr << "Invalid value for --" << flagName << " " << value << "\n";
    return false;
  }
  return true;
}

DEFINE_string(fill_order, "row,col", "Fill by rows or columns");
// DEFINE_validator(fill_order, &validateFillOrder);
DEFINE_int32(play_interval, 3000,
             "Time interval between automatic play actions");
DEFINE_int32(
    stop_after, 9,
    "Stop playing after finishing certain number of rows/columns/blocks");

Player::Player(std::shared_ptr<GameWindow> gameWindow,
               std::shared_ptr<SudokuRecognizer> recognizer,
               std::shared_ptr<SudokuSolver> solver, GameMode gameMode)
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
      playClassic();
      break;
    case GameMode::IRREGULAR:
      playIrregular();
      break;
    case GameMode::ICE_BREAKER:
      playIceBreaker();
      break;
    default:
      LOG(FATAL) << "Unknown game mode " << gameMode_;
  }
}

void Player::playClassic() {
  auto board = solver_->getSolvedBoard();

  // Need to click in the window first to make sure it gets focus
  clickAt(boardRect_.left, boardRect_.top - 50);
  Sleep(3000);  // there is an animation before the screen settles

  // TODO there are probably better ways to implement different fill orders
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
      Sleep(FLAGS_play_interval);
    }
  }
}

void Player::playIrregular() { throw std::runtime_error("Not Implemented"); }

void Player::playIceBreaker() {
  auto board = solver_->getSolvedBoard();
  auto iceBoard = recognizer_->getIceBoard();
  SudokuSolver::printBoard("Completed board", solver_->getCompletedBoard());

  // row, col, weight
  std::vector<std::tuple<int, int, int>> iceLocations;

  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (board[i][j] == 0) {
        iceBoard[i][j] = -1;  // mark existing number locations
      }
      if (iceBoard[i][j] > 0) {
        iceLocations.push_back({i, j, iceBoard[i][j]});
        iceBoard[i][j] = -1;  // mark ice locations
      }
    }
  }

  for (const auto& location : iceLocations) {
    auto row = std::get<0>(location);
    auto col = std::get<1>(location);
    auto weight = std::get<2>(location);
    for (int i = 0; i < 9; i++) {
      if (iceBoard[row][i] < 0 || i == col) {
        continue;
      }
      iceBoard[row][i] += weight;
    }
    for (int i = 0; i < 9; i++) {
      if (iceBoard[i][col] < 0 || i == row) {
        continue;
      }
      iceBoard[i][col] += weight;
    }
  }

  auto iceCellComparator = [](const IceBreakerCell left,
                              const IceBreakerCell right) {
    return std::get<2>(left) < std::get<2>(right);
  };
  std::priority_queue<IceBreakerCell, std::vector<IceBreakerCell>,
                      decltype(iceCellComparator)>
      cells(iceCellComparator);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (iceBoard[i][j] > 0) {
        cells.push(std::make_tuple(i, j, iceBoard[i][j]));
      }
    }
  }

  while (!cells.empty()) {
    auto& cell = cells.top();
    cells.pop();
    auto row = std::get<0>(cell), col = std::get<1>(cell);
    printf("Place %d at (%d, %d) (weight = %d)\n", board[row][col], row + 1,
           col + 1, std::get<2>(cell));
  }
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
}
