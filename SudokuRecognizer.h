#pragma once

#include <vector>
#include <opencv2/core.hpp>
#include <Windows.h>

class SudokuRecognizer {
  public:
    SudokuRecognizer();
    void loadImage(cv::Mat image);
    bool recognize();
    std::vector<std::vector<int>> getResults();
    std::vector<std::pair<int, int>> recognizeIce();
    RECT getBoardRect();

    static void showImage(const cv::Mat& image);

 private:
  cv::Rect findBoard();
  void removeBoundary(cv::Mat& image);
  void clearBoard();

  cv::Mat image_;
  std::vector<std::vector<int>> _board;
  RECT boardRect_;
};