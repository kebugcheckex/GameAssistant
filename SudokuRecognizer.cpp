#include "pch.h"

#include "SudokuRecognizer.h"

#include <tesseract/baseapi.h>

#include <algorithm>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <queue>
#include <random>
#include <unordered_set>

#include "RecognizerUtils.h"
#include "SudokuBoard.h"

// Debug parameters --game-mode irregular --dev-mode --debug --image-file-path
// .\images\Irregular1.png

DECLARE_bool(debug);
DECLARE_bool(dev_mode);

constexpr std::string_view kCvWindowName{"Auto Sudoku"};

const std::vector<cv::Scalar> kDebugColors{
    {255, 0, 0},    // Red
    {0, 255, 0},    // Green
    {0, 0, 255},    // Blue
    {255, 255, 0},  // Yellow
    {0, 255, 255},  // Cyan
    {255, 0, 255},  // Magenta
    {128, 0, 128},  // Purple
    {255, 165, 0},  // Orange
    {0, 128, 128},  // Teal
};

SudokuRecognizer::SudokuRecognizer(GameMode gameMode,
                                   std::shared_ptr<GameWindow> gameWindow)
    : gameMode_(gameMode), gameWindow_(gameWindow) {
  if (FLAGS_debug) {
    cv::namedWindow(kCvWindowName.data());
  }
  image_ = gameWindow->getSnapshot();
}

void SudokuRecognizer::findBoardInWindow() {
  cv::Mat image = image_.clone();
  cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
  cv::threshold(image, image, /* thresh */ 128, /* maxval */ 255,
                cv::THRESH_BINARY);
  std::vector<Contour> contours;
  std::vector<cv::Vec4i> hierachy;
  cv::findContours(image, contours, hierachy, cv::RETR_LIST,
                   cv::CHAIN_APPROX_SIMPLE);
  std::vector<Contour> rectangles;
  for (const auto& contour : contours) {
    Contour approximation;
    cv::approxPolyDP(contour, approximation,
                     cv::arcLength(contour, /* closed */ true) * 0.02,
                     /* closed */ true);
    if (RecognizerUtils::isRectangle(approximation)) {
      rectangles.push_back(approximation);
    }
  }

  // using area to find the board is probably accurate enough and using
  // approxPolyDP and checking for rectangle above is likely unnecessary
  auto boardContour = std::find_if(rectangles.begin(), rectangles.end(),
                                   [](const Contour& contour) {
                                     auto area = cv::contourArea(contour);
                                     // TODO these magic number actually depends
                                     // on the window size, currently hard-coded
                                     // to 1700x1700 in GameWindow.cpp
                                     return area > 757000 && area < 828000;
                                   });

  cvBoardRect_.x = boardContour->at(0).x;
  cvBoardRect_.y = boardContour->at(0).y;
  cvBoardRect_.width = boardContour->at(1).x - boardContour->at(0).x;
  cvBoardRect_.height = boardContour->at(3).y - boardContour->at(0).y;
  RecognizerUtils::printCvRect(cvBoardRect_);

  DCHECK_GT(cvBoardRect_.area(), 757000);
  DCHECK_LT(cvBoardRect_.area(), 828000);

  if (FLAGS_debug) {
    cv::Mat displayImage = image_.clone();
    std::vector<Contour> contoursForDrawing{*boardContour};
    cv::rectangle(displayImage, cvBoardRect_, cv::Scalar(0, 0, 255), 2);

    for (int i = 0; i < boardContour->size(); i++) {
      const auto& point = boardContour->at(i);
      cv::circle(displayImage, point, 10, cv::Scalar(255, 0, 0), 2);
      cv::putText(displayImage, std::to_string(i), point,
                  cv::FONT_HERSHEY_SIMPLEX, 1.f, cv::Scalar(0, 0, 255), 2);
    }
    showImage(displayImage, "BoardRect");
  }
}

void SudokuRecognizer::removeBoundary(cv::Mat& image) {
  // TODO maybe use dilate function is easier
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
  switch (gameMode_) {
    case GameMode::CLASSIC:
    case GameMode::ICE_BREAKER:
      return recognizeClassic();
    case GameMode::IRREGULAR:
      return recognizeIrreguluar();
    default:
      LOG(ERROR) << "Unknown game mode " << gameMode_;
      return false;
  }
}

bool SudokuRecognizer::recognizeClassic() {
  recognizedBoard_ = Board(9, std::vector<int>(9, 0));
  findBoardInWindow();

  cv::Mat boardImage = image_(cvBoardRect_).clone();
  cv::Mat displayImage = boardImage.clone();

  cv::cvtColor(boardImage, boardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(boardImage, boardImage, /* thresh */ 130, /* maxval */ 255,
                cv::THRESH_BINARY);
  removeBoundary(boardImage);

  // offset the thick boundaries by minus 1
  int blockSize = boardImage.rows / 9 - 1;

  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT);
  ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
  ocr->SetVariable("tessedit_char_whitelist", "123456789");

  constexpr int kBoundaryOffset = 7;
  cv::Mat block;
  DOUBLE_FOR_LOOP {
    cv::Rect blockBoundary(blockSize * i + kBoundaryOffset,
                           blockSize * j + kBoundaryOffset, blockSize,
                           blockSize);
    boardImage(blockBoundary).copyTo(block);
    ocr->SetImage(block.data, block.cols, block.rows, 1, block.step);
    auto str = std::string(ocr->GetUTF8Text());
    if (str[0] >= '1' && str[0] <= '9') {
      recognizedBoard_[j][i] = std::stoi(str);
    }
    if (FLAGS_debug) {
      cv::rectangle(displayImage, blockBoundary, cv::Scalar(255, 0, 0));
      cv::putText(displayImage, str.substr(0, 1),
                  cv::Point(blockSize * i + 30, blockSize * j + 30),
                  cv::FONT_HERSHEY_SIMPLEX, 1.f, cv::Scalar(0, 0, 255), 2);
    }
  }
  showImage(displayImage, "OCR image");
  ocr->End();
  return true;
}

bool SudokuRecognizer::recognizeIrreguluar() {
  findBoardInWindow();
  recognizeClassic();
  cv::Mat boardImage = image_(cvBoardRect_).clone();
  cv::cvtColor(boardImage, boardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(boardImage, boardImage, 128, 255, cv::THRESH_BINARY);
  showImage(boardImage, "binary image");
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierachy;
  cv::findContours(boardImage, contours, hierachy, cv::RETR_LIST,
                   cv::CHAIN_APPROX_SIMPLE);

  // The area of a block is the total area of 9 cells. Excluding 10% for the
  // boundary area
  auto blockArea = cvBoardRect_.area() * 0.9 / kDimension;
  std::vector<Contour> blockContours;
  std::copy_if(contours.begin(), contours.end(),
               std::back_inserter(blockContours),
               [blockArea](const Contour& contour) {
                 auto area = cv::contourArea(contour);
                 // add 5% error margin
                 return area > blockArea * 0.95 && area < blockArea * 1.05;
               });
  if (blockContours.size() != 9) {
    LOG(ERROR) << fmt::format(
        "Failed to find the correct number of block contours. Actual number "
        "of contours found: {}\n",
        blockContours.size());
    return false;
  }
  cv::Mat displayImage = image_(cvBoardRect_).clone();
  cv::drawContours(displayImage, blockContours, -1, cv::Scalar(0, 0, 255), 2);
  showImage(displayImage, "Blocks");

  int cellWidth = (int)(cvBoardRect_.width / 9);
  int cellHeight = (int)(cvBoardRect_.height / 9);

  blocks_.resize(kDimension);
  DOUBLE_FOR_LOOP {
    cv::Point2f cellCenter(i * cellHeight + cellHeight / 2.f,
                           j * cellWidth + cellWidth / 2.f);
    bool foundBlock = false;
    int blockId = -1;
    for (int k = 0; k < blockContours.size(); k++) {
      if (cv::pointPolygonTest(blockContours[k], cellCenter, false) > 0.f) {
        blocks_[k].insert(SudokuBoard::convertCoordinateToIndex(j, i));
        foundBlock = true;
        blockId = k;
        break;
      }
    }
    if (!foundBlock) {
      LOG(ERROR) << fmt::format(
          "failed to find which block that cell ({}, {}) belongs to, cell "
          "center is at ({}, {})",
          i, j, cellCenter.x, cellCenter.y);
      return false;
    }
    cv::circle(displayImage, cellCenter, 10, kDebugColors[blockId], cv::FILLED);
  }
  showImage(displayImage, "Blocks with cell center");

  DLOG(INFO) << "===== Blocks Data =====";
  for (const auto& block : blocks_) {
    std::ostringstream oss;
    for (const int index : block) {
      auto [row, col] = SudokuBoard::convertIndexToCoordinate(index);
      oss << fmt::format("({}, {}), ", row, col);
    }
    DLOG(INFO) << oss.str();
  }
  return true;
}

/* static */
cv::Scalar SudokuRecognizer::generateRandomColor() {
  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<int> distribution(0, 255);

  return cv::Scalar(distribution(generator), distribution(generator),
                    distribution(generator));
}

Board SudokuRecognizer::getRecognizedBoard() {
  if (recognizedBoard_.empty()) {
    recognize();
  }
  return recognizedBoard_;
}

Blocks SudokuRecognizer::getBlocks() { return blocks_; }

Board SudokuRecognizer::getIceBoard() { return iceBoard_; }

// Note - the current "ice" images are taken from the game screenshot when the
// window size is fixed to 1700x1700. Template matching may not work if the
// image scales. Need to use things like SIFT
bool SudokuRecognizer::recognizeIce() {
  cv::Mat boardImage;
  image_(cvBoardRect_).copyTo(boardImage);
  cv::Mat displayImage;
  image_(cvBoardRect_).copyTo(displayImage);

  iceBoard_ = Board(9, std::vector<int>(9, 0));
  for (int i = 1; i <= 3; i++) {
    std::stringstream ss;
    ss << "./images/ice" << i << ".png";
    cv::Mat iceImage = cv::imread(ss.str());
    ;
    cv::Mat result;

    // template matching is a bit slow, on slower hardware, we may need to
    // downsample the image before doing template matching
    cv::matchTemplate(boardImage, iceImage, result, cv::TM_CCOEFF_NORMED);
    double threshold = 0.9;
    std::vector<cv::Point> locations;
    cv::findNonZero(result > threshold, locations);
    for (const auto& point : locations) {
      auto x = point.x / 96, y = point.y / 96;
      if (FLAGS_debug) {
        cv::rectangle(
            displayImage, point,
            cv::Point(point.x + iceImage.cols, point.y + iceImage.rows),
            cv::Scalar(0, 0, 255), 2);
        std::stringstream ss;
        ss << "(" << x << ", " << y << ", " << i << ")";
        cv::putText(displayImage, ss.str(), point, cv::FONT_HERSHEY_SIMPLEX, 1,
                    cv::Scalar(200, 0, 200), 2);
      }
      iceBoard_[y][x] = i;
    }
  }
  showImage(displayImage, "ice locations");
  return true;
}

RECT SudokuRecognizer::getBoardRect() {
  RECT rect{};
  rect.left = cvBoardRect_.x;
  rect.top = cvBoardRect_.y;
  rect.right = cvBoardRect_.x + cvBoardRect_.width;
  rect.bottom = cvBoardRect_.y + cvBoardRect_.height;
  return rect;
}

/* static */
void SudokuRecognizer::showImage(const cv::Mat& image,
                                 const std::string& title) {
  if (FLAGS_debug || FLAGS_dev_mode) {
    cv::setWindowTitle(kCvWindowName.data(), title);
    cv::imshow(kCvWindowName.data(), image);
    cv::waitKey();
  }
}