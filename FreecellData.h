#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <opencv2/core.hpp>

namespace game_assistant {
namespace freecell {
enum Suite { CLUB, SPADE, DIAMOND, HEART, INVALID_SUITE };

enum Rank {
  A,
  _2,
  _3,
  _4,
  _5,
  _6,
  _7,
  _8,
  _9,
  _10,
  J,
  Q,
  K,
  INVALID,
};

typedef struct sCard {
  Suite suite;
  Rank rank;

  sCard(int iSuite, int iRank)
      : suite(static_cast<Suite>(iSuite)), rank(static_cast<Rank>(iRank)) {}
} Card;

struct CardHash {
  std::size_t operator()(const Card& card) const {
    return static_cast<size_t>(static_cast<int>(card.suite)) * 100 +
           static_cast<size_t>(card.rank);
  }
};

struct CardEquality {
  bool operator()(const Card& lhs, const Card& rhs) const {
    return lhs.rank == rhs.rank && lhs.suite == rhs.suite;
  }
};

typedef std::vector<Card> Column;

typedef std::vector<Column> Deck;

extern const std::unordered_map<Suite, std::string> kSuiteNames;
extern const std::unordered_map<Suite, std::string> kSuiteSymbols;
extern const std::unordered_map<char, Suite> kCharSuiteMap;
extern const std::unordered_map<char, int> kSuiteOrderMap;
extern const std::unordered_map<char, Rank> kCharRankMap;
extern const std::unordered_map<char, int> kRankOrderMap;
extern const std::unordered_map<Rank, std::string> kRankNames;

int encodeCardLocation(int column, int position);
std::tuple<int, int> decodeCardLocation(int location);

void printCard(const Card& card);
void printDeck(const Deck& deck);

std::string formatCard(const int suite, const int rank);
std::string formatCard(const Suite suite, const Rank rank);
std::string formatCard(const Card& card);

int encodeCard(const Card& card);
int encodeCard(const Suite suite, const Rank rank);
int encodeCard(const int suite, const int rank);

void validateDeck(const Deck& deck);

}  // namespace freecell
}  // namespace game_assistant