#pragma once

#include "FreecellData.h"
#include "GameWindow.h"

namespace GameAssistant {
namespace Freecell {

class FreecellRecognizer {
 public:
  FreecellRecognizer(std::shared_ptr<GameWindow> gameWindow);
  bool recognize();

 private:
  Deck deck_;
  std::shared_ptr<GameWindow> gameWindow_;
};

}  // namespace Freecell
}  // namespace GameAssistant