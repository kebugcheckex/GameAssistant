#include "pch.h"

#include "FreecellData.h"

namespace game_assistant {
namespace freecell {

const std::unordered_map<Suite, std::string> kSuiteNames{
    {Suite::CLUB, "Club"},
    {Suite::SPADE, "Spade"},
    {Suite::DIAMOND, "Diamond"},
    {Suite::HEART, "Heart"},
};

const std::unordered_map<Suite, std::string> kSuiteSymbols{
    {Suite::CLUB, "C"},     // ♣ \u2663
    {Suite::SPADE, "S"},    // ♠ \u2660
    {Suite::DIAMOND, "D"},  // ♦ \u2666
    {Suite::HEART, "H"},    // ♥ \u2665
};

const std::unordered_map<char, Suite> kCharSuiteMap{
    {'C', Suite::CLUB},
    {'S', Suite::SPADE},
    {'D', Suite::DIAMOND},
    {'H', Suite::HEART},
};

const std::unordered_map<char, int> kSuiteOrderMap {
  {'C', 0}, {'S', 1}, {'D', 2}, {'H', 3 },
};

const std::unordered_map<char, Rank> kCharRankMap{
    {'A', Rank::A},  {'2', Rank::_2},  {'3', Rank::_3}, {'4', Rank::_4},
    {'5', Rank::_5}, {'6', Rank::_6},  {'7', Rank::_7}, {'8', Rank::_8},
    {'9', Rank::_9}, {'T', Rank::_10}, {'J', Rank::J},  {'Q', Rank::Q},
    {'K', Rank::K},
};

const std::unordered_map<char, int> kRankOrderMap {
  {'A', 0}, {'2', 1}, {'3', 2}, {'4', 3}, {'5', 4}, {'6', 5}, {'7', 6},
      {'8', 7}, {'9', 8}, {'T', 9}, {'J', 10}, {'Q', 11}, {'K', 12},
};

const std::unordered_map<Rank, std::string> kRankNames{
    {Rank::A, "A"},  {Rank::_2, "2"},  {Rank::_3, "3"}, {Rank::_4, "4"},
    {Rank::_5, "5"}, {Rank::_6, "6"},  {Rank::_7, "7"}, {Rank::_8, "8"},
    {Rank::_9, "9"}, {Rank::_10, "T"}, {Rank::J, "J"},  {Rank::Q, "Q"},
    {Rank::K, "K"},
};

constexpr int kEncodeFactor = 100;
int encodeCardLocation(int column, int position) {
  return column * kEncodeFactor + position;
}

std::tuple<int, int> decodeCardLocation(int location) {
  return {location / kEncodeFactor, location % kEncodeFactor};
}

void printCard(const Card& card) {
  std::cout << kRankNames.at(card.rank) << kSuiteSymbols.at(card.suite) << " ";
}

}  // namespace freecell
}  // namespace game_assistant