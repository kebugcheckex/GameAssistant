#pragma once

#include <string>
#include <opencv2/core.hpp>

class GameWindow {
public:
  GameWindow(const std::string& windowName);
  cv::Mat getSnapshot();

private:
  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
  HWND hwnd_;
};
