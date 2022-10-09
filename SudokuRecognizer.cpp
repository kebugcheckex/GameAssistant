#include "pch.h"
#include "SudokuRecognizer.h"

#include <opencv2/imgproc.hpp>
#include <tesseract/baseapi.h>
#include <queue>
#include <unordered_set>

SudokuRecognizer::SudokuRecognizer() {
  _board = std::vector<std::vector<int>>(9, std::vector<int>(9, 0));
}

void SudokuRecognizer::loadImage(cv::Mat image) {
  _image = image;
}

cv::Rect SudokuRecognizer::findBoard() {
  cv::Mat edgesImage;
  // TODO annotate these parameters or magic numbers
  cv::Canny(_image, edgesImage, 50, 150, 3);
  std::vector<cv::Vec4i> lines;
  // TODO annotate these parameters or magic numbers
  cv::HoughLinesP(edgesImage, lines, 2, CV_PI / 180, 80, 500, 2);
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
    if (starting.x < 50 || starting.y < 50 || ending.x > _image.cols - 50 ||
      starting.y < 300) {
      // exclude lines found outside the board area
      continue;
    }

    // TODO use glog
    printf("Line from (%d, %d) to (%d, %d)\n", starting.x, starting.y, ending.x,
      ending.y);
    linePoints.push_back(starting);
    linePoints.push_back(ending);
  }
  if (linePoints.size() != 8) {
    throw std::runtime_error("Failed to find the board area");
  }

  std::sort(linePoints.begin(), linePoints.end(),
    [](cv::Point a, cv::Point b) { return a.x <= b.x && a.y <= b.y; });
  return cv::Rect(linePoints[0], linePoints[7]);
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
    int index = x * _image.cols + y;
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
}

bool SudokuRecognizer::recognize() {
  auto rect = findBoard();
  cv::Mat boardImage;
  _image(rect).copyTo(boardImage);
  cv::Mat displayImage = _image.clone();
  cv::cvtColor(boardImage, boardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(boardImage, boardImage, 245, 255, cv::THRESH_BINARY);
  std::unordered_set<int> scanned;
  removeBoundary(boardImage);
  // TODO annotate the magic numbers
  int blockSize = boardImage.rows / 9 - 2;
  cv::Mat block;

  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT);
  ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);

  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      boardImage(
        cv::Rect(blockSize * i + 2, blockSize * j + 2, blockSize, blockSize))
        .copyTo(block);
      ocr->SetImage(block.data, block.cols, block.rows, 1, block.step);
      auto str = std::string(ocr->GetUTF8Text());
      if (str[0] >= '1' && str[0] <= '9') {
        _board[j][i] = std::stoi(str);
      }
      cv::putText(displayImage, str.substr(0, 1),
        cv::Point(blockSize * i + 30, blockSize * j + 30),
        cv::FONT_HERSHEY_SIMPLEX, 1.0f, cv::Scalar(0, 0, 255), 2);
    }
  }
  return true;
}

std::vector<std::vector<int>> SudokuRecognizer::getResults() {
  return _board;
}

void SudokuRecognizer::clearBoard() {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      _board[i][j] = 0;
    }
  }
}