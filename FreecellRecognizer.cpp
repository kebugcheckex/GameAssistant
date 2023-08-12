#include "pch.h"

#include "FreecellRecognizer.h"

#include <tesseract/baseapi.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "FreecellData.h"
#include "RecognizerUtils.h"

DEFINE_string(match_threshold, "0.99", "Template matching threshold");

namespace game_assistant {
namespace freecell {

constexpr std::string_view kCardHeadersImagePath{
    "./resources/card_headers.png"};

const std::vector<std::vector<double>> thresholds{
    // A   2    3    4    5    6    7    8    9    T    J    Q    K
    {.90, .90, .91, .90, .94, .92, .93, .96, .93, .90, .90, .90, .85},
    {.95, .90, .94, .94, .93, .94, .95, .96, .93, .90, .88, .85, .90},
    {.90, .90, .90, .94, .93, .95, .90, .93, .93, .90, .80, .80, .80},
    {.90, .95, .95, .93, .93, .96, .95, .90, .90, .90, .92, .90, .90},
};

FreecellRecognizer::FreecellRecognizer(std::shared_ptr<GameWindow> gameWindow)
    : gameWindow_(std::move(gameWindow)) {
  deck_ = Deck(8);
}
bool FreecellRecognizer::recognize_old() {
  constexpr int kThreshold = 180;
  const cv::Point offset(96, 256);
  const cv::Rect decksArea(offset, cv::Point(1470, 640));

  double matchThreshold = std::atof(FLAGS_match_threshold.c_str());

  auto snapshot = gameWindow_->getSnapshot();
  cv::Mat debugImage = snapshot.clone();

  cv::Mat decksImage = snapshot(decksArea);
  // cv::cvtColor(decksImage, decksImage, cv::COLOR_BGR2GRAY);
  // cv::threshold(decksImage, decksImage, kThreshold, 255, cv::THRESH_BINARY);
  utils::showImage(decksImage, "freecell_decks_gray");

  cv::Mat cardImage = cv::imread(R"(.\resources\cards.png)");
  // cv::cvtColor(cardImage, cardImage, cv::COLOR_BGR2GRAY);
  // cv::threshold(cardImage, cardImage, kThreshold, 255, cv::THRESH_BINARY);
  utils::showImage(cardImage, "freecell_card_gray");

  cv::Mat cardSymbol;
  std::vector<CardWithLocation> cards;
  for (int suite = 0; suite < 4; suite++) {
    for (int rank = 0; rank < 13; rank++) {
      cv::Rect symboalArea(rank * kCardSymbolWidth, suite * kCardSymbolHeight,
                           kCardSymbolWidth, kCardSymbolHeight);
      cardImage(symboalArea).copyTo(cardSymbol);

      cv::Mat result;
      cv::matchTemplate(decksImage, cardSymbol, result, cv::TM_CCORR_NORMED);
      // cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
      double minValue, maxValue;
      cv::Point minLocation, maxLocation;
      cv::minMaxLoc(result, &minValue, &maxValue, &minLocation, &maxLocation,
                    cv::Mat());
      std::cout << fmt::format("Matching suite {}, rank {}, value {}\n", suite,
                               rank, maxValue);
      auto& location = maxLocation;
      cv::Rect headerRect(
          location + offset,
          location + offset + cv::Point(cardSymbol.cols, cardSymbol.rows));
      cv::rectangle(debugImage, headerRect,
                    utils::kDebugColors[utils::DebugColors::Red], 2);
      cv::putText(debugImage,
                  kRankNames.at(static_cast<Rank>(rank)) +
                      kSuiteSymbols.at(static_cast<Suite>(suite)),
                  location + cv::Point(-40, 10) + offset,
                  cv::FONT_HERSHEY_SIMPLEX, 0.8,
                  utils::kDebugColors[utils::DebugColors::Yellow], 2);
      cards.push_back(
          {static_cast<Suite>(suite), static_cast<Rank>(rank), location});
    }
  }

  /*cv::Mat cardHeader;
  std::vector<CardWithLocation> cards;
  for (int suite = 0; suite < 4; suite++) {
    for (int rank = 0; rank < 13; rank++) {
      cv::Rect headerArea(suite * kCardHeaderWidth, rank * kCardHeaderHeight,
                          kCardHeaderWidth, kCardHeaderHeight);
      cardHeadersImage(headerArea).copyTo(cardHeader);

      cv::Mat result;
      cv::matchTemplate(decksImage, cardHeader, result, cv::TM_CCORR_NORMED);
      double threshold = 0.99;
      std::vector<cv::Point> locations;
      cv::findNonZero(result > threshold, locations);
      std::cout << fmt::format("Suite {}, Rank {}, Found {} points\n", suite,
                               rank, locations.size());
      if (locations.empty() || locations.size() > 5) {
        continue;
      }
      for (const auto& location : locations) {
        cv::Rect headerRect(
            location + offset,
            location + offset + cv::Point(kCardHeaderWidth, kCardHeaderHeight));
        cv::rectangle(debugImage, headerRect,
            utils::kDebugColors[utils::DebugColors::Red], 2);
        cv::putText(debugImage,
                    kRankNames.at(static_cast<Rank>(rank)) +
                        kSuiteSymbols.at(static_cast<Suite>(suite)),
                    location + cv::Point(-40, 10) + offset,
                    cv::FONT_HERSHEY_SIMPLEX, 0.8,
                    utils::kDebugColors[utils::DebugColors::Yellow], 2);
      }
    }
  }*/

  utils::showImage(debugImage, "freecell_card_locations");
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

bool FreecellRecognizer::recognize() {
  auto snapshot = gameWindow_->getSnapshot();
  cv::Mat debugImage = snapshot.clone();
  cv::cvtColor(snapshot, snapshot, cv::COLOR_BGR2GRAY);
  for (int col = 0; col < 8; col++) {
    int numCards = col < 4 ? 7 : 6;
    for (int row = 0; row < numCards; row++) {
      cv::Rect symbolRect(kFirstCardOffset.x + col * kHorizontalDistance,
                          kFirstCardOffset.y + row * kVerticalDistance,
                          kCardSymbolWidth, kCardSymbolHeight / 2);
      auto cardSymbol = snapshot(symbolRect);
      auto rank = recognizeRank(cardSymbol);
      if (rank == 'X') {
        utils::showImage(cardSymbol, "debug_card_symbol");
      }
      std::cout << fmt::format("Processing col {}, row {}, rank = {}\n", col,
                               row, rank);
      cv::putText(debugImage, std::string(1, rank),
                  symbolRect.tl() + cv::Point(-30, 20),
                  cv::FONT_HERSHEY_SIMPLEX, 0.8,
                  utils::kDebugColors.at(utils::DebugColors::Yellow), 2);
    }
  }

  utils::showImage(debugImage, "freecell_ocr_results");
  return true;
}

char FreecellRecognizer::recognizeRank(cv::Mat& image) {
  tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
  ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT);
  ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
  ocr->SetVariable("debug_file", "NUL");
  ocr->SetVariable("tessedit_char_whitelist", "0123456789AJQK");
  ocr->SetImage(image.data, image.cols, image.rows, 1, image.step);
  auto str = std::string(ocr->GetUTF8Text());
  if (str.empty()) {
    return 'X';
  }
  if (str.at(0) == '1') {
    return 'T';
  } else {
    return str.at(0);
  }
}

void FreecellRecognizer::printCardWithLocation(const CardWithLocation& card) {
  std::cout << kRankNames.at(card.rank) << kSuiteSymbols.at(card.suite) << " ";
}
}  // namespace freecell
}  // namespace game_assistant