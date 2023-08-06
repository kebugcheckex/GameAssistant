#include "pch.h"

#include "GameWindow.h"

#include <gflags/gflags.h>

#include <fstream>
#include <opencv2/imgcodecs.hpp>

#include "CaptureSnapshot.h"

DECLARE_string(image_file);

using namespace winrt::Windows::Graphics::Capture;

GameWindow::GameWindow(const std::string& windowOrFileName) {
  if (!FLAGS_image_file.empty()) {
    imageFromFile_ = cv::imread(windowOrFileName);
    if (imageFromFile_.empty()) {
      LOG(FATAL) << "failed to open image file " << windowOrFileName;
    }
    return;
  }

  // TODO probably should not hardcode FindWindowA here, should use FindWindow
  // instead. That requires additional handling of wstring
  hwnd_ = FindWindowA(NULL, windowOrFileName.c_str());
  if (hwnd_ == NULL) {
    throw std::runtime_error("Failed to find game window");
  }

  // Resize the window to 1700x1700 so that the recognizer can use some
  // hardcoded dimension values
  MoveWindow(hwnd_, 0, 0, 1700, 1700, TRUE);

  if (!GetWindowRect(hwnd_, &windowRect_)) {
    throw std::runtime_error("Failed to get game window rect");
  }

  HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
  MONITORINFO info{0};
  info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(monitor, &info);
  auto screenRect = info.rcMonitor;
  screenWidth_ = screenRect.right - screenRect.left;
  screenHeight_ = screenRect.bottom - screenRect.top;
  DCHECK_GT(screenWidth_, 0);
  DCHECK_GT(screenHeight_, 0);

  auto d3dDevice = robmikh::common::uwp::CreateD3DDevice();
  auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
  device_ = CreateDirect3DDevice(dxgiDevice.get());
}

RECT GameWindow::getWindowRect() {
  if (!imageFromFile_.empty()) {
    LOG(FATAL)
        << "Game window loaded from image file, no window rect available";
  }
  return windowRect_;
}

cv::Mat GameWindow::getSnapshot() {
  if (!imageFromFile_.empty()) {
    return imageFromFile_;
  }
  GraphicsCaptureItem item{nullptr};
  item = robmikh::common::desktop::CreateCaptureItemForWindow(hwnd_);
  auto pixelFormat = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::
      B8G8R8A8UIntNormalized;
  auto coro{CaptureSnapshot::TakeAsync(device_, item, pixelFormat)};
  auto texture = coro.get();  // await sync

  D3D11_TEXTURE2D_DESC desc = {};
  texture->GetDesc(&desc);
  auto bytes = robmikh::common::uwp::CopyBytesFromTexture(texture);

  cv::Mat window(desc.Height, desc.Width, CV_8UC3);

  // For some reason initializing the Mat directly from bytes causes some
  // mysterious errors I have to manually fill the Mat
  int count = 0;
  for (unsigned int i = 0; i < desc.Height; i++) {
    for (unsigned int j = 0; j < desc.Width; j++) {
      // Alpha channel is not needed
      cv::Vec3b pixel{bytes[count], bytes[count + 1], bytes[count + 2]};
      count += 4;
      window.at<cv::Vec3b>(i, j) = pixel;
    }
  }
  return window;
}

void GameWindow::clickAt(int x, int y) {
  INPUT inputs[3] = {};
  inputs[0].type = INPUT_MOUSE;
  inputs[0].mi.dx =
      static_cast<int>((float)(x + windowRect_.left) / screenWidth_ * 65535.f);
  inputs[0].mi.dy =
      static_cast<int>((float)(y + windowRect_.top) / screenHeight_ * 65535.f);
  inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
  inputs[1].type = INPUT_MOUSE;
  inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  inputs[2].type = INPUT_MOUSE;
  inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
  DLOG(INFO) << fmt::format("Clicking at screen coordinate ({}, {})", x, y);
  SendInput(3, inputs, sizeof(INPUT));
}

void GameWindow::pressKey(char ch) {
  INPUT inputs[2] = {};
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = ch;
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = ch;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  DLOG(INFO) << "Pressing key: " << ch;
  SendInput(2, inputs, sizeof(INPUT));
}