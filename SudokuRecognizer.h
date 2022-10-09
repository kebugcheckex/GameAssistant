#pragma once

#include <vector>
#include <opencv2/core.hpp>

class SudokuRecognizer {
  public:
    SudokuRecognizer();
    void loadImage(cv::Mat image);
    bool recognize();
    std::vector<std::vector<int>> getResults();
private:
  cv::Rect findBoard();
  void removeBoundary(cv::Mat& image);
  void clearBoard();

  cv::Mat _image;
  std::vector<std::vector<int>> _board;
};