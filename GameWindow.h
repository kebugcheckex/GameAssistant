#pragma once

#include <string>
#include <string_view>

#include "IGameWindow.h"

// TODO may need i18n in other languages
constexpr std::string_view kGameWindowName{"Microsoft Sudoku"};
constexpr std::string_view kSolitareWindowName{"Solitaire & Casual Games"};

class GameWindow : public IGameWindow {
 public:
  GameWindow(const std::string& windowName);
  cv::Mat getSnapshot() override;

  RECT getWindowRect();

  /*
   * Send a mouse click at (x, y)
   * Note that the coordinate (x, y) is relative to the top-left corner of the
   * game window.
   */
  void clickAt(int x, int y) override;
  void pressKey(char ch) override;  // TODO maybe extend this to other virtual key codes

 private:
  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device_{
      nullptr};
  HWND hwnd_;
  RECT windowRect_;
  std::string windowName_;
  cv::Mat imageFromFile_;
  int screenWidth_, screenHeight_;
  int windowWidth_, windowHeight_;
};
