#pragma once

#include <opencv2/core.hpp>

#include "FreecellData.h"
#include "GameWindow.h"

namespace game_assistant {
namespace freecell {

typedef struct {
  Suite suite;
  Rank rank;
  cv::Point location;
} CardWithLocation;

constexpr int kHorizontalDistance = 172, kVerticalDistance = 48;
constexpr int kCardSymbolWidth = 18, kCardSymbolHeight = 40;
constexpr int kCardHeaderWidth = 118, kCardHeaderHeight = 42;

extern const cv::Point kFirstCardOffset;

class FreecellRecognizer {
 public:
  FreecellRecognizer(std::shared_ptr<GameWindow> gameWindow);
  bool recognize_sift();
  bool recognizeTemplateMatching();
  bool recognizeSuites();
  bool recognize();

 private:
  void printCardWithLocation(const CardWithLocation& card);
  std::pair<int, int> getCardColumnAndRow(const cv::Point& location);
  Deck deck_;
  std::vector<std::vector<cv::Point>> cardLocations_;
  std::shared_ptr<GameWindow> gameWindow_;
};

void generateFreecellTemplate(const std::string& filePath);

}  // namespace freecell
}  // namespace game_assistant