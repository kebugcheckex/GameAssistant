#pragma once

#include "FreecellData.h"
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

class FreecellSpace {
 public:
  FreecellSpace() : freecell_({Suite::CLUB, Rank::INVALID}) {}
  void put(const Card& card) {
    if (freecell_.rank != Rank::INVALID) {
      throw std::runtime_error(
          fmt::format("Freecell is occupied by {}", formatCard(freecell_)));
    }
    freecell_ = card;
  }
  Card get() {
    if (freecell_.rank == Rank::INVALID) {
      throw std::runtime_error("Freecell is empty");
    }
    auto result = freecell_;
    freecell_.rank = Rank::INVALID;
    return result;
  }

 private:
  Card freecell_;
};

class Foundation {
 public:
  Foundation() : suite_(Suite::INVALID_SUITE), top_(Rank::INVALID) {}
  Foundation(Suite suite) : suite_(suite), top_(Rank::INVALID) {}

  void put(const Card& card) {
    if (card.suite != suite_) {
      throw std::runtime_error(
          fmt::format("Invalid move: cannot move {} to Foundation {}",
                      formatCard(card), kSuiteNames.at(suite_)));
    }
    if (card.rank == Rank::A && top_ != Rank::INVALID) {
      throw std::runtime_error(fmt::format(
          "Invalid move: cannot put card {} onto non-empty foundation {}",
          formatCard(card), kSuiteNames.at(suite_)));
    }
    if (card.rank - top_ != 1) {
      throw std::runtime_error(fmt::format(
          "Invalid move: cannot put card {} onto foundation {}, current top is "
          "{}",
          formatCard(card), kSuiteNames.at(suite_), kRankNames.at(top_)));
    }
    top_ = card.rank;
  }
    private:
      Suite suite_;
     Rank top_;
};

typedef struct {
  Deck deck;
  std::vector<FreecellSpace> freecells;
} Board;

typedef struct {
  int numCards;
  Location from, to;
} Move;

typedef std::vector<Move> Solution;

class FreecellPlayer {
 public:
  explicit FreecellPlayer(std::shared_ptr<IGameWindow> gameWindow);
  void play(const std::string& solutionsFilePath);

  static void printMoves(const std::vector<Move>& moves);

  static void validateSolution(const Solution& solution);

 private:
  std::shared_ptr<IGameWindow> gameWindow_;
};

}  // namespace freecell
}  // namespace game_assistant
