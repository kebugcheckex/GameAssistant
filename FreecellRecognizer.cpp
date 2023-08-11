#include "pch.h"

#include "FreecellRecognizer.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "FreecellData.h"
#include "RecognizerUtils.h"

namespace game_assistant {
namespace freecell {

constexpr std::string_view kCardSymbolsImagePath{"./resources/cards.png"};
constexpr int kCardSymbolWidth = 18;
constexpr int kCardSymbolHeight = 40;

FreecellRecognizer::FreecellRecognizer(std::shared_ptr<GameWindow> gameWindow)
    : gameWindow_(std::move(gameWindow)) {
  deck_ = Deck(8);
}
bool FreecellRecognizer::recognize() {
  constexpr int kThreshold = 130;
  const cv::Point offset(96, 256);

  auto snapshot = gameWindow_->getSnapshot();
  cv::Mat cardArea = snapshot(cv::Rect(offset, cv::Point(1400, 640)));

  cv::Mat redChannel, greenChannel, blueChannel, binaryImage;
  cv::extractChannel(cardArea, redChannel, 2);
  cv::extractChannel(cardArea, greenChannel, 1);
  cv::extractChannel(cardArea, blueChannel, 0);
  cv::threshold(redChannel, binaryImage, /* thresh */ kThreshold,
                /* maxval */ 255, cv::THRESH_BINARY);
  utils::showImage(binaryImage, "freecell_binary_image");

  cv::Mat displayImage = snapshot.clone();

  /*std::vector<Contour> contours;
  std::vector<cv::Vec4i> hierachy;
  cv::findContours(binaryImage, contours, hierachy, cv::RETR_LIST,
                   cv::CHAIN_APPROX_SIMPLE);
  utils::sortContourByArea(contours, true);
  contours.resize(8);
  for (const auto& contour : contours) {
    cv::drawContours(displayImage, contours, -1,
                     utils::kDebugColors[utils::DebugColors::Cyan], 2);
  }
  utils::showImage(displayImage, "freecell_contours");
  return true;*/

  cv::Mat cardSymbolsImage = cv::imread(kCardSymbolsImagePath.data());
  cv::Mat binarySymbolsImage;
  cv::cvtColor(cardSymbolsImage, binarySymbolsImage, cv::COLOR_BGR2GRAY);
  cv::threshold(binarySymbolsImage, binarySymbolsImage, /* thresh */ kThreshold,
                /* maxval */ 255, cv::THRESH_BINARY);
  // utils::showImage(binarySymbolsImage, "freecell_binary_symbol_image");

  cv::Mat cardSymbol, debugImage;
  snapshot(cv::Rect(cv::Point(96, 256), cv::Point(1400, 640)))
      .copyTo(debugImage);
  std::vector<CardWithLocation> cards;
  for (int suite = 0; suite < 4; suite++) {
    for (int rank = 0; rank < 13; rank++) {
      cv::Rect symbolLocation(rank * kCardSymbolWidth,
                              suite * kCardSymbolHeight, kCardSymbolWidth,
                              kCardSymbolHeight);
      binarySymbolsImage(symbolLocation).copyTo(cardSymbol);
      cv::Mat result;
      // TODO this is a bit slow, use multi-threads
      cv::matchTemplate(binaryImage, cardSymbol, result, cv::TM_CCOEFF_NORMED);
      double threshold = 0.9;
      std::vector<cv::Point> locations;
      cv::findNonZero(result > threshold, locations);
      std::cout << fmt::format("Suite {}, Rank {}, Found {} points\n", suite,
                               rank, locations.size());
      if (locations.empty()) {
        continue;
      }
      for (const auto& location : locations) {
        cv::rectangle(displayImage, location + offset,
                      location + cv::Point(16, 40) + offset,
                      utils::kDebugColors[utils::DebugColors::Red], 2);
        cv::putText(displayImage,
                    kRankNames.at(static_cast<Rank>(rank)) +
                        kSuiteSymbols.at(static_cast<Suite>(suite)),
                    location + cv::Point(-40, 10) + offset,
                    cv::FONT_HERSHEY_SIMPLEX, 0.8,
                    utils::kDebugColors[utils::DebugColors::Blue], 2);
        cards.push_back({static_cast<Suite>(suite), static_cast<Rank>(rank),
                         location + offset});
      }
      utils::showImage(displayImage, "freecell_card_locations");
    }
  }

  std::sort(cards.begin(), cards.end(),
            [](auto& a, auto& b) { return a.location.x < b.location.x; });

  int index = 0;
  for (int col = 0; col < 8; col++) {
    int numCards = col < 4 ? 7 : 6;
    std::sort(cards.begin() + index, cards.begin() + index + numCards,
              [](auto& a, auto& b) { return a.location.y < b.location.y; });
    for (size_t i = index; i < index + numCards; i++) {
      auto& card = cards.at(i);
      std::cout << kRankNames.at(card.rank) << kSuiteSymbols.at(card.suite)
                << " ";
    }
    index += numCards;
    std::cout << "\n";
  }
  DCHECK_EQ(index, 52);
  return true;
}

void FreecellRecognizer::printCardWithLocation(const CardWithLocation& card) {
  std::cout << kRankNames.at(card.rank) << kSuiteSymbols.at(card.suite) << " ";
}
}  // namespace freecell
}  // namespace game_assistant