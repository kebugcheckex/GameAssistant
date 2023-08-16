#pragma once

#include "IGameWindow.h"

namespace game_assistant {
namespace freecell {

enum LocationType { Stack, Freecell, Foundation };

const std::unordered_map<LocationType, std::string> kLocationTypeNames{
    {LocationType::Stack, "Stack"},
    {LocationType::Freecell, "Freecell"},
    {LocationType::Foundation, "Foundation"}};

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
  explicit FreecellPlayer(std::shared_ptr<IGameWindow> gameWindow);
  void play(const std::string& solutionsFilePath);

  static void printMoves(const std::vector<Move>& moves);

 private:
  std::shared_ptr<IGameWindow> gameWindow_;
};

}  // namespace freecell
}  // namespace game_assistant
