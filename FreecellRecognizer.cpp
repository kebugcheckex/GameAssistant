#include "pch.h"

#include "FreecellRecognizer.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "FreecellData.h"
#include "RecognizerUtils.h"

namespace GameAssistant {
namespace Freecell {
constexpr std::string_view kCardSymbolsImagePath{"./resources/cards.png"};
constexpr int kCardSymbolWidth = 18;
constexpr int kCardSymbolHeight = 40;

FreecellRecognizer::FreecellRecognizer(std::shared_ptr<GameWindow> gameWindow)
    : gameWindow_(std::move(gameWindow)) {
  deck_ = Deck(8);
}
bool FreecellRecognizer::recognize() {
  auto snapshot = gameWindow_->getSnapshot();

  cv::Mat binaryImage;
  cv::cvtColor(snapshot, binaryImage, cv::COLOR_BGR2GRAY);
  cv::threshold(binaryImage, binaryImage, /* thresh */ 200, /* maxval */ 255,
                cv::THRESH_BINARY);
  Utils::showImage(binaryImage, "freecell_binary_image");

  std::vector<Contour> contours;
  std::vector<cv::Vec4i> hierachy;
  cv::findContours(binaryImage, contours, hierachy, cv::RETR_LIST,
                   cv::CHAIN_APPROX_SIMPLE);
  Utils::sortContourByArea(contours, true);
  contours.resize(8);
  // TODO convert these contours to vectors
  // then locate each card
  cv::Mat displayImage = snapshot.clone();
  for (const auto& contour : contours) {
    cv::drawContours(displayImage, contours, -1,
                     Utils::kDebugColors[Utils::DebugColors::Cyan], 2);
  }
  Utils::showImage(displayImage, "freecell_contours");
  return true;

  cv::Mat cardSymbolsImage = cv::imread(kCardSymbolsImagePath.data());
  cv::Mat binarySymbolsImage;
  cv::cvtColor(cardSymbolsImage, binarySymbolsImage, cv::COLOR_BGR2GRAY);
  cv::threshold(binarySymbolsImage, binarySymbolsImage, /* thresh */ 200,
                /* maxval */ 255, cv::THRESH_BINARY);
  Utils::showImage(binarySymbolsImage, "freecell_binary_symbol_image");

  cv::Mat cardSymbol;

  for (int suite = 0; suite < 4; suite++) {
    for (int rank = 0; rank < 13; rank++) {
      cv::Rect symbolLocation(rank * kCardSymbolWidth,
                              suite * kCardSymbolHeight, kCardSymbolWidth,
                              kCardSymbolHeight);
      binarySymbolsImage(symbolLocation).copyTo(cardSymbol);
      cv::Mat result;
      cv::matchTemplate(binaryImage, cardSymbol, result, cv::TM_CCOEFF_NORMED);
      double threshold = 0.9;
      std::vector<cv::Point> locations;
      cv::findNonZero(result > threshold, locations);
      std::cout << fmt::format("Suite {}, Rank {}, Found {} points\n", suite,
                               rank, locations.size());
      DCHECK_EQ(locations.size(), 1);
      cv::rectangle(displayImage, locations.at(0),
                    locations.at(0) + cv::Point(16, 40),
                    Utils::kDebugColors[Utils::DebugColors::Red], 2);
      cv::putText(displayImage,
                  kRankNames.at(static_cast<Rank>(rank)) +
                      kSuiteSymbols.at(static_cast<Suite>(suite)),
                  locations.at(0) - cv::Point(60, -10),
                  cv::FONT_HERSHEY_SIMPLEX, 0.8,
                  Utils::kDebugColors[Utils::DebugColors::Blue], 2);
    }
  }
  Utils::showImage(displayImage, "freecell_card_locations");
  return true;
}
}  // namespace Freecell
}  // namespace GameAssistant