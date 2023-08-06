#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <string_view>

// TODO may need i18n in other languages
constexpr std::string_view kGameWindowName{"Microsoft Sudoku"};
constexpr uint32_t kWindowWidth = 1200;
constexpr uint32_t kWindowHeight = 900;
constexpr uint32_t kWindowArea = kWindowWidth * kWindowHeight;

class GameWindow {
 public:
  GameWindow(const std::string& windowName);
  cv::Mat getSnapshot();

  RECT getWindowRect();

  /*
   * Send a mouse click at (x, y)
   * Note that the coordinate (x, y) is relative to the top-left corner of the
   * game window.
   */
  void clickAt(int x, int y);
  void pressKey(char ch);  // TODO maybe extend this to other virtual key codes

 private:
  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device_{
      nullptr};
  HWND hwnd_;
  RECT windowRect_;
  std::string windowName_;
  cv::Mat imageFromFile_;
  int screenWidth_, screenHeight_;
};
