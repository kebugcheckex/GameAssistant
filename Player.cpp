#include "pch.h"
#include "Player.h"

#include <fmt/core.h>
#include <glog/logging.h>

DEFINE_string(fill_order, "rows?,cols?,blocks?",
              "Fill ? rows/cols/blocks, e.g. rows3 means fill 3 rows");
DEFINE_int32(play_interval, 3000,
           "Time interval between automatic play actions");
DEFINE_int32(
    stop_after, 9,
    "Stop playing after finishing certain number of rows/columns/blocks");

Player::Player(const RECT& monitorRect, const RECT& boardRect) {
  screenWidth_ = monitorRect.right - monitorRect.left;
  screenHeight_ = monitorRect.bottom - monitorRect.top;
  gridSize_ = (boardRect.right - boardRect.left) / 9;
  boardRect_ = boardRect;
  DLOG(INFO) << fmt::format("Board Rect: ({}, {}) -> ({}, {})\n", boardRect.left,
                           boardRect.top, boardRect.right, boardRect.bottom);
}


void Player::play(const std::vector<std::vector<int>>& board) {
  // Need to click in the window first to make sure it gets focus
  // And there is also an animation
  clickAt(boardRect_.left, boardRect_.top - 50);
  Sleep(3000);
  // TODO there are probably better ways to implement different fill orders
  for (int i = 0; i < FLAGS_stop_after; i++) {
    for (int j = 0; j < 9; j++) {
      if (FLAGS_fill_order == "rows") {
        if (board[i][j] == 0) {
          continue;
        }
        fillAt(i, j, (char)board[i][j]);
      } else if (FLAGS_fill_order == "cols") {
        if (board[j][i] == 0) {
          continue;
        }
        fillAt(j, i, (char)board[j][i]);
      } 
      Sleep(FLAGS_play_interval);
    }
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
