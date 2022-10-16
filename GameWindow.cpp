#include "pch.h"
#include "GameWindow.h"
#include "CaptureSnapshot.h"

#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <gflags/gflags.h>

DECLARE_bool(dev_mode);

using namespace winrt::Windows::Graphics::Capture;

GameWindow::GameWindow(const std::string& windowOrFileName) {
  if (FLAGS_dev_mode) {
    imageFromFile_ = cv::imread(windowOrFileName);
    return;
  }

  // TODO probably should not hardcode FindWindowA here, should use FindWindow instead.
  // That requires additional handling of wstring
  hwnd_ = FindWindowA(NULL, windowOrFileName.c_str());
  if (hwnd_ == NULL) {
    throw std::runtime_error("Failed to find game window");
  }

  // Resize the window to 1700x1700 so that the recognizer can use some hardcoded dimension values
  MoveWindow(hwnd_, 0, 0, 1700, 1700, TRUE);

  if (!GetWindowRect(hwnd_, &windowRect_)) {
    throw std::runtime_error("Failed to get game window rect");
  }

  auto d3dDevice = robmikh::common::uwp::CreateD3DDevice();
  auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
  device_ = CreateDirect3DDevice(dxgiDevice.get());
}

RECT GameWindow::getWindowRect() { 
  if (FLAGS_dev_mode) {
    throw std::runtime_error(
        "Game window loaded from image file, no window rect available");
  }
  return windowRect_;
}

RECT GameWindow::getMonitorRect() {
  if (FLAGS_dev_mode) {
    throw std::runtime_error(
        "Game window loaded from image file, no monitor rect available");
  }
  HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
  MONITORINFO info;
  info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(monitor, &info);
  return info.rcMonitor;
}

cv::Mat GameWindow::getSnapshot() {
  if (FLAGS_dev_mode) {
    return imageFromFile_;
  }
  GraphicsCaptureItem item{ nullptr };
  item = robmikh::common::desktop::CreateCaptureItemForWindow(hwnd_);
  auto pixelFormat = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;
  auto coro{ CaptureSnapshot::TakeAsync(device_, item, pixelFormat) };
  auto texture = coro.get();  // await sync

  D3D11_TEXTURE2D_DESC desc = {};
  texture->GetDesc(&desc);
  auto bytes = robmikh::common::uwp::CopyBytesFromTexture(texture);
  
  cv::Mat window(desc.Height, desc.Width, CV_8UC4);

  // For some reason initializing the Mat directly from bytes causes some mysterious errors
  // I have to manually fill the Mat
  int count = 0;
  for (unsigned int i = 0; i < desc.Height; i++) {
    for (unsigned int j = 0; j < desc.Width; j++) {
      cv::Vec4b pixel{ bytes[count], bytes[count + 1], bytes[count + 2], bytes[count + 3] };
      count += 4;
      window.at<cv::Vec4b>(i, j) = pixel;
    }
  }
  return window;
}