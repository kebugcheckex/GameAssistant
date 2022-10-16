#pragma once

#include <string>
#include <string_view>
#include <opencv2/core.hpp>

// TODO may need i18n in other languages
constexpr std::string_view kWindowName{ "Microsoft Sudoku" };

class GameWindow {
public:
  GameWindow(const std::string& windowName);
  cv::Mat getSnapshot();
  RECT getWindowRect();
  RECT getMonitorRect();

private:
  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device_{ nullptr };
  HWND hwnd_;
  RECT windowRect_;
  std::string windowName_;
  cv::Mat imageFromFile_;
};
