#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <opencv2/core.hpp>

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

constexpr int kHorizontalDistance = 172, kVerticalDistance = 48;
constexpr int kCardSymbolWidth = 18, kCardSymbolHeight = 44;
constexpr int kCardHeaderWidth = 118, kCardHeaderHeight = 42;

extern const cv::Point kFirstCardOffset;

void printCard(const Card& card);

void generateFreecellTemplate(const std::string& filePath);

}  // namespace freecell
}  // namespace game_assistant