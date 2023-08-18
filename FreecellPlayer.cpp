#include "pch.h"

#include "FreecellPlayer.h"

#include <fmt/core.h>

#include <fstream>
#include <iostream>
#include <regex>

namespace game_assistant {
namespace freecell {
FreecellPlayer::FreecellPlayer(std::shared_ptr<IGameWindow> gameWindow)
    : gameWindow_(gameWindow) {}

void FreecellPlayer::play(const std::string& solutionsFilePath) {
  std::ifstream solutionsFile(solutionsFilePath);
  std::string line;

  std::regex pattern(
      "Move ([0-9a]) cards? from (stack|freecell) (\\d) to "
      "(stack|freecell|the foundation)( \\d)?");

  std::vector<Move> moves;

  while (std::getline(solutionsFile, line)) {
    if (line.find("Move") == std::string::npos) {
      continue;
    }
    std::smatch sm;
    std::regex_search(line, sm, pattern);
    if (sm.size() < 4) {
      LOG(ERROR) << "Invalid line: " << line;
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
    try {
      fromId = std::stoi(sm.str(3));
    } catch (std::exception& ex) {
      LOG(ERROR) << fmt::format("Failed to stoi {} due to {}", sm.str(3),
                                ex.what());
      continue;
    }

    if (sm.str(4).find("stack") != std::string::npos) {
      toType = LocationType::Stack;
    } else if (sm.str(4).find("freecell") != std::string::npos) {
      toType = LocationType::Freecell;
    } else if (sm.str(4).find("the foundation") != std::string::npos) {
      toType = LocationType::Foundation;
    } else {
      LOG(ERROR) << "Bad line " << line;
      continue;
    }

    toId = toType == LocationType::Foundation ? -1 : std::stoi(sm.str(5));

    moves.push_back({numCards, {fromType, fromId}, {toType, toId}});
  }

  printMoves(moves);
}

// static
void FreecellPlayer::printMoves(const std::vector<Move>& moves) {
  for (const auto& move : moves) {
    std::cout << fmt::format("{} cards: {} {} => {} {}\n", move.numCards,
                             kLocationTypeNames.at(move.from.type),
                             move.from.id, kLocationTypeNames.at(move.to.type),
                             move.to.type != LocationType::Foundation
                                 ? std::to_string(move.to.id)
                                 : "");
  }
}

// static
void FreecellPlayer::validateSolution(const Solution& solution) {

}
}  // namespace freecell
}  // namespace game_assistant
