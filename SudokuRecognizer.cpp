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

DECLARE_bool(debug);
DECLARE_bool(dev_mode);

constexpr std::string_view kCvWindowName{"Auto Sudoku"};

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
  // the edges of the board countour may not be horizontal/vertical
  // these four lines is a workaround to make them horizontal/vertical
  //boardContour->at(1).y = boardContour->at(0).y;
  //boardContour->at(3).x = boardContour->at(0).x;
  //boardContour->at(2).x = boardContour->at(1).x;
  //boardContour->at(2).y = boardContour->at(3).y;

  cvBoardRect_.x = boardContour->at(0).x;
  cvBoardRect_.y = boardContour->at(0).y;
  cvBoardRect_.width = boardContour->at(1).x - boardContour->at(0).x;
  cvBoardRect_.height = boardContour->at(3).y - boardContour->at(0).y;
  RecognizerUtils::printCvRect(cvBoardRect_);

 /* DCHECK_GT(cvBoardRect_.area(), 757000);
  DCHECK_LT(cvBoardRect_.area(), 828000);*/

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
  }
}

bool SudokuRecognizer::recognizeClassic() {
  recognizedBoard_ = Board(9, std::vector<int>(9, 0));
  findBoardInWindow();

  cv::Mat boardImage;
  image_(cvBoardRect_).copyTo(boardImage);
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
        recognizedBoard_[j][i] = std::stoi(str);
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

  if (gameMode_ == GameMode::ICE_BREAKER) {
    recognizeIce();
  }
  return true;
}

bool SudokuRecognizer::recognizeIrreguluar() {
  findBoardInWindow();
  return true;
  cv::Mat croppedImage = image_.clone();
  // image_(cv::Rect(0, 350, 960, 940)).copyTo(croppedImage);
  cv::Mat grayImage;
  cv::cvtColor(croppedImage, grayImage, cv::COLOR_BGR2GRAY);
  cv::Mat binaryImage;
  cv::threshold(grayImage, binaryImage, 128, 255, cv::THRESH_BINARY);
  // cv::Canny(binaryImage, binaryImage, 0 /* threshold1 */,
  //           150 /* threshold2 */);
  showImage(binaryImage, "binary image");
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierachy;
  cv::findContours(binaryImage, contours, hierachy, cv::RETR_LIST,
                   cv::CHAIN_APPROX_SIMPLE);

  // Find the board
  std::vector<Contour> result;
  std::copy_if(contours.begin(), contours.end(), std::back_inserter(result),
               [](const Contour& contour) {
                 auto area = cv::contourArea(contour);
                 return area > 757000 && area < 828000;
               });
  cv::drawContours(image_, result, -1, cv::Scalar(0, 0, 255), 2);
  showImage(image_, "result");

  // TODO
  // 1. move the above logic to findBoard and also record the offset from the
  // original image
  // 2. design a data structure to store irregular blocks
  // 3. find those irregular blocks

  // for (int i = 0; i < contours.size(); i++) {
  //   const auto& contour = contours[i];
  //   if (contour.size() > 18) {
  //     continue;
  //   }
  //   cv::Mat displayImage = croppedImage.clone();
  //   std::cout << fmt::format("Contour #{}: size = {}\n", i, contour.size());
  //   cv::drawContours(displayImage, contours, i, cv::Scalar(0, 255, 0), 2);
  //   for (int j = 0; j < contour.size(); j++) {
  //     cv::circle(displayImage, contour[j], 10, cv::Scalar(0, 0, 255), 2);
  //   }
  //   showImage(displayImage, "contours");
  // }
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