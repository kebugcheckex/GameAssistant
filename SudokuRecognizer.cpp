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

DECLARE_bool(debug);

namespace game_assistant {
namespace sudoku {
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
  std::vector<utils::Contour> contours;
  std::vector<cv::Vec4i> hierachy;
  cv::findContours(image, contours, hierachy, cv::RETR_LIST,
                   cv::CHAIN_APPROX_SIMPLE);
  std::vector<utils::Contour> rectangles;
  for (const auto& contour : contours) {
    utils::Contour approximation;
    cv::approxPolyDP(contour, approximation,
                     cv::arcLength(contour, /* closed */ true) * 0.02,
                     /* closed */ true);
    if (utils::isRectangle(approximation)) {
      rectangles.push_back(approximation);
    }
  }
  game_assistant::utils::sortContourByArea(rectangles, true);
  if (FLAGS_debug) {
    cv::Mat debugImage = image_.clone();
    cv::drawContours(debugImage, rectangles, -1, cv::Scalar(0, 0, 255), 2);
    /* for (const auto& contour : rectangles) {
      cv::Point textLocation(
          contour[0].x + Recognizerutils::getRandomInt(-30, 30),
          contour[0].y + Recognizerutils::getRandomInt(-30, 30));
      cv::line(debugImage, contour[0], textLocation, cv::Scalar(255, 0, 255),
               2);
      cv::putText(debugImage,
                  std::to_string(static_cast<int>(cv::contourArea(contour))),
                  textLocation, cv::FONT_HERSHEY_SIMPLEX, 0.5,
                  cv::Scalar(255, 0, 255), 1);
      DLOG(INFO) << "Contour area " << cv::contourArea(contour);
    }*/
    showImage(debugImage, "findBoardInWindow - rectangle contours");
  }

  // Assuming the board is the second largest rectangle in the window, while the
  // first being the whole window client area. This is not rigorous!
  // If this doesn't work, use the commented code snippet instead.
  auto boardContour = rectangles.begin() + 1;
  /* auto boardContour = std::find_if(
       rectangles.begin(), rectangles.end(), [](const Contour& contour) {
         auto area = cv::contourArea(contour);
         return area > kWindowArea * 0.33 && area < kWindowArea * 0.35;
       });
   if (boardContour == rectangles.end()) {
     LOG(FATAL) << "failed to find the board contour";
   }*/
  boardRect_.x = boardContour->at(0).x;
  boardRect_.y = boardContour->at(0).y;
  boardRect_.width = boardContour->at(1).x - boardContour->at(0).x;
  boardRect_.height = boardContour->at(3).y - boardContour->at(0).y;
  utils::printCvRect(boardRect_);

  if (FLAGS_debug) {
    cv::Mat displayImage = image_.clone();
    std::vector<utils::Contour> contoursForDrawing{*boardContour};
    cv::rectangle(displayImage, boardRect_, cv::Scalar(0, 0, 255), 2);

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

  cv::Mat boardImage = image_(boardRect_).clone();
  cv::Mat displayImage = boardImage.clone();

  cv::cvtColor(boardImage, boardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(boardImage, boardImage,
                /* thresh */ gameMode_ == GameMode::ICE_BREAKER ? 250 : 192,
                /* maxval */ 255, cv::THRESH_BINARY);
  removeBoundary(boardImage);

  // offset the thick boundaries by minus 1
  int blockSize = boardImage.rows / 9 - 1;

  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT);
  ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
  ocr->SetVariable("debug_file", "NUL");
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
  recognizeClassic();
  cv::Mat boardImage = image_(boardRect_).clone();
  cv::cvtColor(boardImage, boardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(boardImage, boardImage, /* thresh */ 128, /* maxval */ 255,
                cv::THRESH_BINARY);
  showImage(boardImage, "recognizeIrreguluar: binary image");
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierachy;
  cv::findContours(boardImage, contours, hierachy, cv::RETR_LIST,
                   cv::CHAIN_APPROX_SIMPLE);

  // The area of a block is the total area of 9 cells. Excluding 10% for the
  // boundary area
  auto blockArea = boardRect_.area() * 0.9 / kDimension;
  std::vector<utils::Contour> blockContours;
  std::copy_if(contours.begin(), contours.end(),
               std::back_inserter(blockContours),
               [blockArea](const utils::Contour& contour) {
                 auto area = cv::contourArea(contour);
                 // add 6% error margin
                 return area > blockArea * 0.94 && area < blockArea * 1.06;
               });

  cv::Mat displayImage = image_(boardRect_).clone();
  for (int i = 0; i < kDimension; i++) {
    cv::drawContours(displayImage, blockContours, i,
                     game_assistant::utils::kDebugColors[i], 2);
  }
  showImage(displayImage, "recognizeIrreguluar block contours");

  if (blockContours.size() != 9) {
    LOG(ERROR) << fmt::format(
        "Failed to find the correct number of block contours. Actual number "
        "of contours found: {}\n",
        blockContours.size());
    return false;
  }

  int cellWidth = (int)(boardRect_.width / 9);
  int cellHeight = (int)(boardRect_.height / 9);

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
    cv::circle(displayImage, cellCenter, 20,
               game_assistant::utils::kDebugColors[blockId], cv::FILLED);
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
    if (!recognize()) {
      LOG(FATAL) << "failed to recognize board";
    }
  }
  return recognizedBoard_;
}

Blocks SudokuRecognizer::getBlocks() { return blocks_; }

Board SudokuRecognizer::getIceBoard() {
  if (iceBoard_.empty()) {
    if (!recognizeIce()) {
      LOG(FATAL) << "failed to recognize ice board";
    }
  }
  return iceBoard_;
}

bool SudokuRecognizer::recognizeIce() {
  cv::Mat boardImage;
  image_(boardRect_).copyTo(boardImage);
  cv::Mat displayImage;
  image_(boardRect_).copyTo(displayImage);

  iceBoard_ = Board(9, std::vector<int>(9, 0));
  for (int iceLevel = 1; iceLevel <= 3; iceLevel++) {
    std::stringstream ss;
    cv::Mat iceImage =
        cv::imread(fmt::format("./resources/ice{}.png", iceLevel));
    double scale = boardRect_.width / kScaleReference;
    cv::resize(iceImage, iceImage, cv::Size(), scale, scale);

    cv::Mat result;
    cv::matchTemplate(boardImage, iceImage, result, cv::TM_CCOEFF_NORMED);
    double threshold = 0.9;
    std::vector<cv::Point> locations;
    cv::findNonZero(result > threshold, locations);

    int cellWidth = (int)(boardRect_.width / 9);
    int cellHeight = (int)(boardRect_.height / 9);

    for (const auto& point : locations) {
      auto x = point.x / cellWidth, y = point.y / cellHeight;
      if (FLAGS_debug) {
        cv::rectangle(
            displayImage, point,
            cv::Point(point.x + iceImage.cols, point.y + iceImage.rows),
            cv::Scalar(0, 0, 255), 2);
        auto text = fmt::format("({}, {}) - {}", x, y, iceLevel);
        cv::Point textLocation(point.x, point.y + cellHeight / 2);
        cv::putText(displayImage, text, textLocation, cv::FONT_HERSHEY_SIMPLEX,
                    0.4, cv::Scalar(200, 0, 200), 1);
      }
      iceBoard_[y][x] = iceLevel;
    }
  }
  showImage(displayImage, "ice locations");
  return true;
}

cv::Rect SudokuRecognizer::getBoardRect() { return boardRect_; }

/* static */
void SudokuRecognizer::showImage(const cv::Mat& image,
                                 const std::string& title) {
  if (FLAGS_debug) {
    cv::setWindowTitle(kCvWindowName.data(), title);
    cv::imshow(kCvWindowName.data(), image);
    char ch = cv::waitKey();
    switch (ch) {
      case 'q':
        exit(0);
      case 's':
        auto fileName = fmt::format("./images/{}.png", title);
        cv::imwrite(fileName, image);
        LOG(INFO) << "image saved to " << fileName;
        break;
    }
  }
}

}  // namespace sudoku
}  // namespace game_assistant
