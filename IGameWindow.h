#pragma once

#include <opencv2/core.hpp>

class IGameWindow {
 public:
  virtual cv::Mat getSnapshot() = 0;
  virtual void clickAt(int x, int y) = 0;
  virtual void pressKey(char ch) = 0;
};
