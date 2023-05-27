#include "pch.h"
#include "Player.h"

#include <fmt/core.h>
#include <glog/logging.h>

DEFINE_string(fill_order, "rows?,cols?,blocks?",
              "Fill ? rows/cols/blocks, e.g. rows3 means fill 3 rows");

bool Player::shouldStop_ = false;

Player::Player(const RECT& monitorRect, const RECT& boardRect) {
  screenWidth_ = monitorRect.right - monitorRect.left;
  screenHeight_ = monitorRect.bottom - monitorRect.top;
  gridSize_ = (boardRect.right - boardRect.left) / 9;
  boardRect_ = boardRect;
  DLOG(INFO) << fmt::format("Board Rect: ({}, {}) -> ({}, {})\n", boardRect.left,
                           boardRect.top, boardRect.right, boardRect.bottom);
}


void Player::play(const std::vector<std::vector<int>>& board) {
  HHOOK keyboardHook =
      SetWindowsHookExA(WH_KEYBOARD_LL, Player::handleHotKey, NULL, 0);
  if (keyboardHook == NULL) {
    DWORD dw = GetLastError();
    LOG(ERROR)
        << "Failed to register keyboard hook. Hotkey is not available.\n";
    LOG(ERROR) << "Error code " << dw << "\n";
  }

  // Need to click in the window first to make sure it gets focus
  // And there is also an animation
  clickAt(boardRect_.left, boardRect_.top - 50);
  Sleep(3000);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (shouldStop_) {
        LOG(INFO) << "Received notification to stop playing";
        break;
      }
      if (board[i][j] == 0) {
        continue;
      }
      fillAt(i, j, (char)board[i][j]);
      Sleep(1000);
    }
    if (shouldStop_) {
      break;
    }
  }
  if (keyboardHook != NULL) {
    UnhookWindowsHookEx(keyboardHook);
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

// static
LRESULT CALLBACK Player::handleHotKey(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    DLOG(INFO) << "Received hot key event";
    if (wParam == WM_KEYUP) {
      LPKBDLLHOOKSTRUCT data = (LPKBDLLHOOKSTRUCT)lParam;
      DLOG(INFO) << "Key code is " << data->vkCode;
      if (data->vkCode == VK_F1 || data->vkCode == VK_NUMPAD0) {
        shouldStop_ = true;
        DLOG(INFO) << "Stoping now!";
      }
    }
  }
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}
