#pragma once

#include <Windows.h>

#include <opencv2/core.hpp>
#include <vector>

#include "GameWindow.h"
#include "SudokuData.h"

namespace game_assistant {
namespace sudoku {
constexpr double kScaleReference = 896.;

class SudokuRecognizer {
 public:
  SudokuRecognizer(GameMode gameMode, std::shared_ptr<GameWindow> gameWindow);
  bool recognize();

  /*
   * Returns the board from the game
   */
  Board getRecognizedBoard();

  /*
   * Get the blocks layout. For irregular mode only. Otherwise returns an empty
   * vector.
   */
  Blocks getBlocks();

  /*
   * Ice Board is a 2D vector where non-zero values reporesents ice in that
   * location The value of the cell means how "hard" the ice is, i.e. how many
   * times it takes to eliminate the ice.
   */
  Board getIceBoard();

  cv::Rect getBoardRect();

  static void showImage(const cv::Mat& image, const std::string& title);
  // util functions
  static cv::Scalar generateRandomColor();

 private:
  bool recognizeIrreguluar();
  bool recognizeClassic();
  void findBoardInWindow();
  void removeBoundary(cv::Mat& image);
  bool recognizeIce();

  cv::Mat image_;
  Board recognizedBoard_, iceBoard_;
  cv::Rect boardRect_;
  GameMode gameMode_;
  Blocks blocks_;
  std::shared_ptr<GameWindow> gameWindow_;
};

}  // namespace sudoku
}  // namespace game_assistant
