#include "pch.h"

#include "RecognizerUtils.h"

#include <fmt/core.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <random>

#include "FreecellData.h"

namespace game_assistant {
namespace utils {

constexpr double EPS = 1e-6;

double calculateCosineAngle(cv::Point vertex, cv::Point side1,
                            cv::Point side2) {
  /*
   * \cos(\theta) =
   * \frac{\mathbf{a}\mathbf{b}}{\lvert\mathbf{a}\rvert\lvert\mathbf{b}\rvert}
   */
  double dx1 = side1.x - vertex.x;
  double dy1 = side1.y - vertex.y;
  double dx2 = side2.x - vertex.x;
  double dy2 = side2.y - vertex.y;
  return (dx1 * dx2 + dy1 * dy2) /
         std::sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

std::vector<cv::Rect> findRectangles(const std::vector<Contour>& contours,
                                     bool sortByAreaDesc) {
  std::vector<cv::Rect> rectangles;
  bool isRectangle = true;
  for (const auto& contour : contours) {
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx,
                     cv::arcLength(contour, /* closed */ true) * 0.02,
                     /* closed */ true);
    if (approx.size() != 4) {
      continue;
    }
    for (int i = 0; i < 4; i++) {
      auto angle = calculateCosineAngle(contour[(i + 1) % 4], contour[i],
                                        contour[(i + 2) % 4]);
      if (fabs(angle - 90. > EPS)) {
        isRectangle = false;
        break;
      }
    }
    if (isRectangle) {
      rectangles.emplace_back(approx[0], approx[2]);
    }
  }

  if (sortByAreaDesc) {
    std::sort(rectangles.begin(), rectangles.end(), [](cv::Rect a, cv::Rect b) {
      return (a.width * a.height) < (b.width * b.height);
    });
  }
  return rectangles;
}

bool isRectangle(const Contour& contour, bool rotated) {
  if (contour.size() != 4) {
    return false;
  }
  if (rotated) {
    for (int i = 0; i < 4; i++) {
      auto cosineAngle = calculateCosineAngle(contour[(i + 1) % 4], contour[i],
                                              contour[(i + 2) % 4]);
      if (cosineAngle > 0.1) {  // about 6 degrees error margin
        return false;
      }
    }
    return true;
  } else {
    // TODO figure out the best way to check if the rectangle is vertical or
    // horizontal
    return true;
  }
}

cv::Rect convertContourToRect(const Contour& contour) {
  DCHECK_EQ(contour.size(), 4);
  auto vertices = contour;
  std::sort(vertices.begin(), vertices.end(),
            [](const cv::Point& a, const cv::Point& b) {
              return a.x <= b.x && a.y <= b.y;
            });
  // TODO this is flawed, doesn't work for rotated rectangles
  return cv::Rect(vertices[0], vertices[3]);
}

void printCvRect(const cv::Rect& rect) {
  LOG(INFO) << fmt::format("Rectangle ({}, {}) -> ({}, {})\n", rect.x, rect.y,
                           rect.x + rect.width, rect.y + rect.height);
}

int getRandomInt(int min, int max) {
  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<int> distribution(min, max);
  return distribution(generator);
}

// TODO some of the orders are wrong, they are in RGB, fix them
const std::vector<cv::Scalar> kDebugColors{
    {0, 0, 255},    // Red
    {0, 255, 0},    // Green
    {255, 0, 0},    // Blue
    {0, 255, 255},  // Yellow
    {255, 255, 0},  // Cyan
    {255, 0, 255},  // Magenta
    {128, 0, 128},  // Purple
    {255, 165, 0},  // Orange
    {0, 128, 128},  // Teal
};

void showImage(const cv::Mat& image, const std::string& title) {
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

void sortContourByArea(std::vector<Contour>& contours, bool descending) {
  std::sort(contours.begin(), contours.end(),
            [descending](const Contour& a, const Contour& b) {
              auto areaA = cv::contourArea(a);
              auto areaB = cv::contourArea(b);
              return descending ? (areaA > areaB) : (areaA < areaB);
            });
}

void generateFreecellTemplate(const std::string& filePath) {
  auto screenshot = cv::imread(filePath);
  if (screenshot.empty()) {
    LOG(FATAL) << fmt::format("Failed to read image file {}", filePath);
  }
  constexpr std::string_view kWindowName{"Freecell Template"};
  cv::namedWindow(kWindowName.data());

  constexpr std::string_view kOutputWindowName { "Freecell Output" };
  cv::namedWindow(kOutputWindowName.data());
  const int dx = 172, dy = 47, width = 118, height = 42;
  cv::Mat output(height * 13, width * 4, CV_8UC3);
  const cv::Point offset(130, 278);

  int count = 1;
  for (int col = 0; col < 8; col++) {
    int numCards = col < 4 ? 7 : 6;
    for (int row = 0; row < numCards; row++) {
      cv::Rect sourceRect(offset.x + col * dx, offset.y + row * dy, width,
                          height);
      cv::imshow(kWindowName.data(), screenshot(sourceRect));
      cv::waitKey(100);
      std::string card;
      std::cout << fmt::format("({}/52) Enter card: ", count++);
      std::cin >> card;  // e.g. JH, 3C
      int outputRow = freecell::kRankOrderMap.at(card.at(0));
      int outputCol = freecell::kSuiteOrderMap.at(card.at(1));
      cv::Rect destinationRect(outputCol * width, outputRow * height, width,
                               height);
      screenshot(sourceRect).copyTo(output(destinationRect));
      cv::imshow(kOutputWindowName.data(), output);
    }
  }
  cv::imshow(kWindowName.data(), output);
  cv::waitKey(0);
  cv::imwrite(R"(.\images\card_headers.png)", output);
}
}  // namespace utils
}  // namespace game_assistant