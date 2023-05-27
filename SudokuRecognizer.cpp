#include "pch.h"
#include "SudokuRecognizer.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <tesseract/baseapi.h>
#include <glog/logging.h>
#include <queue>
#include <fmt/core.h>
#include <unordered_set>

DECLARE_bool(debug);

SudokuRecognizer::SudokuRecognizer() {
  _board = std::vector<std::vector<int>>(9, std::vector<int>(9, 0));
}

void SudokuRecognizer::loadImage(cv::Mat image) {
  image_ = image;
}

cv::Rect SudokuRecognizer::findBoard() {
  cv::Mat edgesImage;
  cv::Canny(image_, edgesImage, 50 /* threshold1 */, 150 /* threshold2 */);
  std::vector<cv::Vec4i> lines;
  cv::HoughLinesP(edgesImage, lines, 2 /* rho */, CV_PI / 180 /* theta */,
                  80 /* threshold */, 500 /* minLineLength  */,
                  2 /* maxLineGap */);
  std::vector<cv::Point> linePoints;
  for (const auto& line : lines) {
    cv::Point starting, ending;
    if (line[0] <= line[2] && line[1] <= line[3]) {
      starting.x = line[0];
      starting.y = line[1];
      ending.x = line[2];
      ending.y = line[3];
    }
    else {
      starting.x = line[2];
      starting.y = line[3];
      ending.x = line[0];
      ending.y = line[1];
    }
    if (starting.x < 50 || starting.y < 50 || ending.x > image_.cols - 50 ||
      starting.y < 300) {
      // exclude lines found outside the board area
      // TODO this depends on the window size, need to use relative locations
      continue;
    }

    /*LOG(INFO) << fmt::format("Line from ({}, {}) to ({}, {})\n", starting.x, starting.y, ending.x,
      ending.y);*/
    linePoints.push_back(starting);
    linePoints.push_back(ending);
  }
  if (linePoints.size() != 8) {
    throw std::runtime_error("Failed to find the board area");
  }

  std::sort(linePoints.begin(), linePoints.end(),
    [](cv::Point a, cv::Point b) { return a.x <= b.x && a.y <= b.y; });
  boardRect_.left = linePoints[0].x;
  boardRect_.top = linePoints[0].y;
  boardRect_.right = linePoints[7].x;
  boardRect_.bottom = linePoints[7].y;
  
  cv::Rect boardRect(linePoints[0], linePoints[7]);
  if (FLAGS_debug) {
    cv::Mat displayImage = image_.clone();
    cv::rectangle(displayImage, boardRect, cv::Scalar(0, 0, 255), 2);
    showImage(displayImage);
  }
  return boardRect;
}

void SudokuRecognizer::removeBoundary(cv::Mat& image) {
  std::unordered_set<int> scanned;
  std::queue<cv::Point> pending;
  pending.push(cv::Point(1, 1));
  while (!pending.empty()) {
    auto& point = pending.front();
    pending.pop();
    int x = point.x, y = point.y;

    if (x < 0 || x >= image.rows || y < 0 || y >= image.cols) {
      continue;
    }
    int index = x * image_.cols + y;
    if (scanned.find(index) != scanned.end()) {
      continue;
    }
    scanned.insert(index);
    if (image.at<uchar>(x, y) == 255) {
      continue;
    }
    image.at<uchar>(x, y) = 255;
    pending.push(cv::Point(x + 1, y));
    pending.push(cv::Point(x, y + 1));
    pending.push(cv::Point(x - 1, y));
    pending.push(cv::Point(x, y - 1));
  }
  if (FLAGS_debug) {
    showImage(image);
  }
}

bool SudokuRecognizer::recognize() {
  auto rect = findBoard();
  cv::Mat boardImage;
  image_(rect).copyTo(boardImage);
  cv::Mat displayImage = boardImage.clone();
  cv::cvtColor(boardImage, boardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(boardImage, boardImage, 245, 255, cv::THRESH_BINARY);
  std::unordered_set<int> scanned;
  removeBoundary(boardImage);

  // offset the thick boundaries by minus 1
  int blockSize = boardImage.rows / 9 - 1;
  
  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT);
  ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
  ocr->SetVariable("tessedit_char_whitelist", "123456789");

  constexpr int kBoundaryOffset = 7;
  cv::Mat block;
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      cv::Rect blockBoundary(blockSize * i + kBoundaryOffset,
                             blockSize * j + kBoundaryOffset, blockSize,
                             blockSize);
      boardImage(blockBoundary)
        .copyTo(block);
      ocr->SetImage(block.data, block.cols, block.rows, 1, block.step);
      auto str = std::string(ocr->GetUTF8Text());
      if (str[0] >= '1' && str[0] <= '9') {
        _board[j][i] = std::stoi(str);
      } else {
        printf("Warning: could not recognize cell (%d, %d), result is %s\n", j,
               i, str.c_str());
      }
      #ifdef _DEBUG
      cv::rectangle(displayImage, blockBoundary, cv::Scalar(255, 0, 0));
      cv::putText(displayImage, str.substr(0, 1),
        cv::Point(blockSize * i + 30, blockSize * j + 30),
        cv::FONT_HERSHEY_SIMPLEX, 1.f, cv::Scalar(0, 0, 255), 2);
      #endif
    }
  }
  if (FLAGS_debug) {
    showImage(displayImage);
  }
  return true;
}

std::vector<std::vector<int>> SudokuRecognizer::getResults() {
  return _board; }

std::vector<std::pair<int, int>> SudokuRecognizer::recognizeIce() {
  auto rect = findBoard();
  cv::Mat boardImage;
  image_(rect).copyTo(boardImage);
  cv::threshold(boardImage, boardImage, 220, 255, cv::THRESH_BINARY);

  return std::vector<std::pair<int, int>>();
}

void SudokuRecognizer::clearBoard() {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      _board[i][j] = 0;
    }
  }
}

RECT SudokuRecognizer::getBoardRect() { return boardRect_; }

/* static */
void SudokuRecognizer::showImage(const cv::Mat& image) {
  cv::imshow(kCvWindowName.data(), image);
  cv::waitKey();
}