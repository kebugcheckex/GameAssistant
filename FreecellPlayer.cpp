#include "pch.h"

#include "FreecellPlayer.h"

#include <fstream>
#include <regex>

namespace game_assistant {
namespace freecell {
FreecellPlayer::FreecellPlayer(std::shared_ptr<GameWindow> gameWindow)
    : gameWindow_(gameWindow) {}

void FreecellPlayer::play(const std::string& solutionsFilePath) {
  std::ifstream solutionsFile(solutionsFilePath);
  std::string line;

  std::regex pattern(
      "Move ([0-9a]) cards? from ((stack|freecell)) (\\d) to "
      "(stack|freecell|foundation)( \\d)?");

  while (std::getline(solutionsFile, line)) {
    if (line.find("Move") == std::string::npos) {
      continue;
    }
    std::smatch sm;
    std::regex_search(line, sm, pattern);
    if (sm.size() < 4) {
      LOG(FATAL) << "Invalid line: " << line;
    }
    int numCards;
    if (sm.str(1) == "a") {
      numCards = 1;
    } else {
      numCards = sm.str(1).at(0) - '0';
    }
    LocationType fromType, toType;
    int fromId, toId;
    if (sm.str(2).find("stack") != std::string::npos) {
      fromType = LocationType::Stack;
    } else if (sm.str(2).find("freecell") != std::string::npos) {
      fromType = LocationType::Freecell;
    } else {
      LOG(FATAL) << "Invalid line: " << line;
    }
    fromId = std::stoi(sm.str(3));

    if (sm.str(4).find("stack") != std::string::npos) {
      toType = LocationType::Stack;
    } else if (sm.str(4).find("freecell") != std::string::npos) {
      toType = LocationType::Freecell;
    } else if (sm.str(4).find("foundation") != std::string::npos) {
      toType = LocationType::Foundation;
    }

    toId = toType == LocationType::Foundation ? -1 : std::stoi(sm.str(5));
  }
}
}  // namespace freecell
}  // namespace game_assistant
