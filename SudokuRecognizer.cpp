#include "pch.h"
#include "SudokuRecognizer.h"

#include <fmt/core.h>
#include <glog/logging.h>
#include <tesseract/baseapi.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <queue>
#include <unordered_set>


DECLARE_bool(debug);
DECLARE_bool(dev_mode);

constexpr std::string_view kCvWindowName{"Auto Sudoku"};

SudokuRecognizer::SudokuRecognizer(GameMode gameMode): gameMode_(gameMode) {
  _board = Board(9, std::vector<int>(9, 0));
  if (FLAGS_debug) {
    cv::namedWindow(kCvWindowName.data());
  }
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
    } else {
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

    LOG(INFO) << fmt::format("Line from ({}, {}) to ({}, {})\n", starting.x,
                             starting.y, ending.x, ending.y);
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

  cvBoardRect_ = cv::Rect(linePoints[0], linePoints[7]);
  if (FLAGS_debug) {
    cv::Mat displayImage = image_.clone();
    cv::rectangle(displayImage, cvBoardRect_, cv::Scalar(0, 0, 255), 2);
    showImage(displayImage, "board rect");
  }
  return cvBoardRect_;
}

void SudokuRecognizer::removeBoundary(cv::Mat& image) {
  showImage(image, "before removed boundary");
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
  showImage(image, "after removed boundary");
}

bool SudokuRecognizer::recognize() {
  auto rect = findBoard();
  cv::Mat boardImage;
  image_(rect).copyTo(boardImage);
  cv::Mat displayImage = boardImage.clone();
  cv::cvtColor(boardImage, boardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(boardImage, boardImage, 250, 255, cv::THRESH_BINARY);
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
      boardImage(blockBoundary).copyTo(block);
      ocr->SetImage(block.data, block.cols, block.rows, 1, block.step);
      auto str = std::string(ocr->GetUTF8Text());
      if (str[0] >= '1' && str[0] <= '9') {
        _board[j][i] = std::stoi(str);
      } else {
        printf("Warning: could not recognize cell (%d, %d), result is %s\n", j,
               i, str.c_str());
      }
      if (FLAGS_debug) {
        cv::rectangle(displayImage, blockBoundary, cv::Scalar(255, 0, 0));
        cv::putText(displayImage, str.substr(0, 1),
                    cv::Point(blockSize * i + 30, blockSize * j + 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1.f, cv::Scalar(0, 0, 255), 2);
      }
    }
  }
  showImage(displayImage, "OCR image");
  ocr->End();
  return true;
}

Board SudokuRecognizer::getResults() { return _board; }


// Note - the current "ice" images are taken from the game screenshot when the window size
// is fixed to 1700x1700. Template matching may not work if the image scales. Need to use
// things like SIFT
Board SudokuRecognizer::recognizeIce() {
  cv::Mat boardImage;
  image_(cvBoardRect_).copyTo(boardImage);
  cv::Mat displayImage;
  image_(cvBoardRect_).copyTo(displayImage);

  Board results(9, std::vector<int>(9, 0));
  for (int i = 1; i <= 3; i++) {
    std::stringstream ss;
    ss << "./images/ice" << i << ".png";
    printf("Image name %s\n", ss.str().c_str());
    cv::Mat iceImage = cv::imread(ss.str());;
    cv::Mat result;

    // template matching is a bit slow, on slower hardware, we may need to downsample the image
    // before doing template matching
    cv::matchTemplate(boardImage, iceImage, result, cv::TM_CCOEFF_NORMED);
    double threshold = 0.9;
    std::vector<cv::Point> locations;
    cv::findNonZero(result > threshold, locations);
    for (const auto& point : locations) {
      auto x = point.x / 96, y = point.y / 96;
      if (FLAGS_debug) {
        cv::rectangle(displayImage, point,
            cv::Point(point.x + iceImage.cols, point.y + iceImage.rows),
                      cv::Scalar(0, 0, 255), 2);
        std::stringstream ss;
        ss << "(" << x << ", " << y << ", " << i << ")";
        cv::putText(displayImage, ss.str(), point, cv::FONT_HERSHEY_SIMPLEX, 1,
                    cv::Scalar(200, 0, 200), 2);
      }
      results[y][x] = i;
    }
  }
  showImage(displayImage, "ice locations");
  return results;
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
void SudokuRecognizer::showImage(const cv::Mat& image, const std::string& title) {
  if (FLAGS_debug || FLAGS_dev_mode) {
    cv::setWindowTitle(kCvWindowName.data(), title);
    cv::imshow(kCvWindowName.data(), image);
    cv::waitKey();
  }
}