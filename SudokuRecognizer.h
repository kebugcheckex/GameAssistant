#pragma once

#include <Windows.h>

#include <opencv2/core.hpp>
#include <vector>

#include "Defs.h"
#include "GameWindow.h"

class SudokuRecognizer {
 public:
  SudokuRecognizer(GameMode gameMode, std::shared_ptr<GameWindow> gameWindow);
  bool recognize();

  /*
   * Returns the board from the game
   */
  Board getRecognizedBoard();

  /*
   * Ice Board is a 2D vector where non-zero values reporesents ice in that
   * location The value of the cell means how "hard" the ice is, i.e. how many
   * times it takes to eliminate the ice.
   */
  Board getIceBoard();
  RECT getBoardRect();

  static void showImage(const cv::Mat& image, const std::string& title);
  // util functions
  static cv::Scalar generateRandomColor();
  static std::vector<cv::Rect> findRectangles(
      const std::vector<std::vector<cv::Point>>& contours);

 private:
  bool recognizeIrreguluar();
  bool recognizeClassic();
  cv::Rect findBoard();
  void removeBoundary(cv::Mat& image);
  bool recognizeIce();

  
  cv::Mat image_;
  Board recognizedBoard_, iceBoard_;
  RECT boardRect_;
  cv::Rect cvBoardRect_;
  GameMode gameMode_;
  std::shared_ptr<GameWindow> gameWindow_;
};