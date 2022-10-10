#include "pch.h"

#include "Player.h"

Player::Player(const RECT& monitorRect) { 
  screenWidth_ = monitorRect.right - monitorRect.left;
  screenHeight_ = monitorRect.bottom - monitorRect.top;
}

void Player::play(const RECT& boardRect,
                  const std::vector<std::vector<int>>& board) {
  int gridSize = (boardRect.right - boardRect.left) / 9;
  printf("Board Rect: (%d, %d) -> (%d, %d)\n", boardRect.left, boardRect.top,
         boardRect.right, boardRect.bottom);
  // Need to click in the window first to make sure it gets focus
  // And there is also an animation
  clickAt(boardRect.left, boardRect.top - 50);
  Sleep(3000);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      if (board[i][j] == 0) {
        continue;
      }
      int x = boardRect.left + j * gridSize + gridSize / 2;
      int y = boardRect.top + i * gridSize + gridSize / 2;
      clickAt(x, y);
      Sleep(1000);
      pressKey('0' + board[i][j]);
      Sleep(2000);
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
  printf("Clicking at (%d, %d)\n", x, y);
  SendInput(3, inputs, sizeof(INPUT));
}

void Player::pressKey(char ch) {
  INPUT inputs[2] = {};
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = ch;
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = ch;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  printf("Pressing key %c\n", ch);
  SendInput(2, inputs, sizeof(INPUT));
}