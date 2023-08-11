#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace game_assistant {
namespace freecell {
enum Suite { CLUB, SPADE, DIAMOND, HEART };

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
};

typedef struct {
  Suite suite;
  Rank rank;
} Card;

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

}  // namespace freecell
}  // namespace game_assistant