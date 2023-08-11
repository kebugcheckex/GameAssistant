#pragma once

#include "FreecellData.h"
#include "GameWindow.h"

namespace game_assistant {
namespace freecell {

typedef struct {
  Suite suite;
  Rank rank;
  cv::Point location;
} CardWithLocation;

class FreecellRecognizer {
 public:
  FreecellRecognizer(std::shared_ptr<GameWindow> gameWindow);
  bool recognize();

 private:
  void printCardWithLocation(const CardWithLocation& card);
  Deck deck_;
  std::shared_ptr<GameWindow> gameWindow_;
};

}  // namespace freecell
}  // namespace game_assistant