#pragma once

#include <vector>
#include <opencv2/core.hpp>
#include <Windows.h>

#include "Defs.h"

class SudokuRecognizer {
  public:
    SudokuRecognizer(GameMode gameMode);
    void loadImage(cv::Mat image);
    bool recognize();
    Board getResults();
    /* The returned structure is a tuple of 3 ints. (x, y, ice-break-level).
     * ice-break-level is how many times an ice need to be hit in order to be eliminated */
    Board recognizeIce();
    RECT getBoardRect();

    static void showImage(const cv::Mat& image, const std::string& title);

 private:
  cv::Rect findBoard();
  void removeBoundary(cv::Mat& image);
  void clearBoard();

  cv::Mat image_;
  Board _board;
  RECT boardRect_;
  cv::Rect cvBoardRect_;
  GameMode gameMode_;
};