#include "pch.h"

#include "FreecellData.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

namespace game_assistant {
namespace freecell {

const std::unordered_map<Suite, std::string> kSuiteNames{
    {Suite::CLUB, "Club"},
    {Suite::SPADE, "Spade"},
    {Suite::DIAMOND, "Diamond"},
    {Suite::HEART, "Heart"},
};

// TODO figure out how to use Unicode symbols
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

const std::unordered_map<char, int> kSuiteOrderMap{
    {'C', 0},
    {'S', 1},
    {'D', 2},
    {'H', 3},
};

const std::unordered_map<char, Rank> kCharRankMap{
    {'A', Rank::A},  {'2', Rank::_2},  {'3', Rank::_3}, {'4', Rank::_4},
    {'5', Rank::_5}, {'6', Rank::_6},  {'7', Rank::_7}, {'8', Rank::_8},
    {'9', Rank::_9}, {'T', Rank::_10}, {'J', Rank::J},  {'Q', Rank::Q},
    {'K', Rank::K},
};

const std::unordered_map<char, int> kRankOrderMap{
    {'A', 0}, {'2', 1}, {'3', 2}, {'4', 3},  {'5', 4},  {'6', 5},  {'7', 6},
    {'8', 7}, {'9', 8}, {'T', 9}, {'J', 10}, {'Q', 11}, {'K', 12},
};

const std::unordered_map<Rank, std::string> kRankNames{
    {Rank::A, "A"},  {Rank::_2, "2"},  {Rank::_3, "3"}, {Rank::_4, "4"},
    {Rank::_5, "5"}, {Rank::_6, "6"},  {Rank::_7, "7"}, {Rank::_8, "8"},
    {Rank::_9, "9"}, {Rank::_10, "T"}, {Rank::J, "J"},  {Rank::Q, "Q"},
    {Rank::K, "K"},
};

constexpr int kEncodeFactor = 100;
const cv::Point kFirstCardOffset(130, 278);

const std::string kReferenceData =
    "5C 4H 2H 3D 6C 8C 2D"
    "6H 9H 7C 9S 4D KD KC"
    "KS 6S 6D JC JD 3C TD"
    "5D AH 8S AS JH JS QD"
    "8H QC 7H 5H TH QS"
    "2C 9D AC 2S 3H AD"
    "7D 9C 4S QH TS KH"
    "3S 4C TC 5S 8D 7S";

int encodeCardLocation(int column, int position) {
  return column * kEncodeFactor + position;
}

std::tuple<int, int> decodeCardLocation(int location) {
  return {location / kEncodeFactor, location % kEncodeFactor};
}

void printCard(const Card& card) {
  if (card.rank == Rank::INVALID) {
    std::cout << "XX ";
  } else {
    std::cout << kRankNames.at(card.rank) << kSuiteSymbols.at(card.suite)
              << " ";
  }
}

void printDeck(const Deck& deck) {
  for (const auto& column : deck) {
    for (const auto& card : column) {
      printCard(card);
    }
    std::cout << "\n";
  }
}

std::string formatCard(const int suite, const int rank) {
  return formatCard(static_cast<Suite>(suite), static_cast<Rank>(rank));
}

std::string formatCard(const Suite suite, const Rank rank) {
  return fmt::format("{}{}", kRankNames.at(rank), kSuiteSymbols.at(suite));
}

void generateFreecellTemplate(const std::string& filePath) {
  auto screenshot = cv::imread(filePath);
  if (screenshot.empty()) {
    LOG(FATAL) << fmt::format("Failed to read image file {}", filePath);
  }
  constexpr std::string_view kWindowName{"Freecell Template"};
  cv::namedWindow(kWindowName.data());

  constexpr std::string_view kOutputWindowName{"Freecell Output"};
  cv::namedWindow(kOutputWindowName.data());
  cv::Mat output(kCardHeaderHeight * 13, kCardHeaderWidth * 4, CV_8UC3);

  int count = 1;
  for (int col = 0; col < 8; col++) {
    int numCards = col < 4 ? 7 : 6;
    for (int row = 0; row < numCards; row++) {
      cv::Rect sourceRect(kFirstCardOffset.x + col * kHorizontalDistance,
                          kFirstCardOffset.y + row * kVerticalDistance,
                          kCardHeaderWidth, kCardHeaderHeight);
      cv::imshow(kWindowName.data(), screenshot(sourceRect));
      cv::waitKey(100);
      std::string card;
      std::cout << fmt::format("({}/52) Enter card: ", count++);
      std::cin >> card;  // e.g. JH, 3C
      int outputRow = freecell::kRankOrderMap.at(card.at(0));
      int outputCol = freecell::kSuiteOrderMap.at(card.at(1));
      cv::Rect destinationRect(outputCol * kCardHeaderWidth,
                               outputRow * kCardHeaderHeight, kCardHeaderWidth,
                               kCardHeaderHeight);
      screenshot(sourceRect).copyTo(output(destinationRect));
      cv::imshow(kOutputWindowName.data(), output);
    }
  }
  cv::imshow(kWindowName.data(), output);
  cv::waitKey(0);
  cv::imwrite(R"(.\images\card_headers.png)", output);
}

}  // namespace freecell
}  // namespace game_assistant