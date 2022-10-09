#include "pch.h"
#include "GameWindow.h"
#include "CaptureSnapshot.h"

#include <fstream>

using namespace winrt::Windows::Graphics::Capture;

GameWindow::GameWindow(const std::string& windowName) {
  hwnd_ = FindWindow(NULL, L"Microsoft Sudoku");
  if (hwnd_ == NULL) {
    throw std::runtime_error("Failed to find window");
  }

  auto d3dDevice = robmikh::common::uwp::CreateD3DDevice();
  auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
  m_device = CreateDirect3DDevice(dxgiDevice.get());
}

cv::Mat GameWindow::getSnapshot() {
  GraphicsCaptureItem item{ nullptr };
  item = robmikh::common::desktop::CreateCaptureItemForWindow(hwnd_);
  auto pixelFormat = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;
  auto coro{ CaptureSnapshot::TakeAsync(m_device, item, pixelFormat) };
  auto texture = coro.get();  // await sync

  D3D11_TEXTURE2D_DESC desc = {};
  texture->GetDesc(&desc);
  auto bytes = robmikh::common::uwp::CopyBytesFromTexture(texture);
  
  cv::Mat window(desc.Height, desc.Width, CV_8UC4);

  // For some reason initializing the Mat directly from bytes causes some mysterious errors
  // I have to manually fill the Mat
  int count = 0;
  for (int i = 0; i < desc.Height; i++) {
    for (int j = 0; j < desc.Width; j++) {
      cv::Vec4b pixel{ bytes[count], bytes[count + 1], bytes[count + 2], bytes[count + 3] };
      count += 4;
      window.at<cv::Vec4b>(i, j) = pixel;
    }
  }
  return window;
}