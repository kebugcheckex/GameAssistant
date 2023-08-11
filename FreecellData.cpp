#include "pch.h"

#include "FreecellData.h"
namespace GameAssistant {
namespace Freecell {

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

const std::unordered_map<Rank, std::string> kRankNames{
    {Rank::A, "A"},  {Rank::_2, "2"},   {Rank::_3, "3"}, {Rank::_4, "4"},
    {Rank::_5, "5"}, {Rank::_6, "6"},   {Rank::_7, "7"}, {Rank::_8, "8"},
    {Rank::_9, "9"}, {Rank::_10, "10"}, {Rank::J, "J"},  {Rank::Q, "Q"},
    {Rank::K, "K"},
};

constexpr int kEncodeFactor = 100;
int encodeCardLocation(int column, int position) {
  return column * kEncodeFactor + position;
}

std::tuple<int, int> decodeCardLocation(int location) {
  return {location / kEncodeFactor, location % kEncodeFactor};
}

}  // namespace Freecell
}  // namespace GameAssistant