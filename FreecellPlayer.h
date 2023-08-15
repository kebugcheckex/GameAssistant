#pragma once

#include "GameWindow.h"

namespace game_assistant {
namespace freecell {

enum LocationType { Stack, Freecell, Foundation };

typedef struct {
  LocationType type;
  int id;
} Location;

typedef struct {
  int numCards;
  Location from, to;
} Move;

class FreecellPlayer {
 public:
  explicit FreecellPlayer(std::shared_ptr<GameWindow> gameWindow);
  void play(const std::string& solutionsFilePath);

 private:
  std::shared_ptr<GameWindow> gameWindow_;
};

}  // namespace freecell
}  // namespace game_assistant
