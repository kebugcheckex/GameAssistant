#include "pch.h"

#include "FreecellRecognizer.h"

#include <tesseract/baseapi.h>

#include <opencv2/features2d.hpp>
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
  for (int col = 0; col < 8; col++) {
    int numCards = col < 4 ? 7 : 6;
    Column column(numCards, Card{Suite::CLUB, Rank::INVALID});
    deck_.push_back(column);
    std::vector<cv::Point> locations(numCards);
    cardLocations_.push_back(locations);
  }
}

bool FreecellRecognizer::recognizeTemplateMatching() {
  constexpr int kThreshold = 180;
  const cv::Rect decksArea(kFirstCardOffset, cv::Point(1470, 640));

  double matchThreshold = std::atof(FLAGS_match_threshold.c_str());

  auto snapshot = gameWindow_->getSnapshot();
  cv::Mat debugImage = snapshot.clone();

  cv::Mat decksImage = snapshot(decksArea);
  cv::cvtColor(decksImage, decksImage, cv::COLOR_BGR2GRAY);
  cv::threshold(decksImage, decksImage, kThreshold, 255, cv::THRESH_BINARY);
  utils::showImage(decksImage, "freecell_decks_gray");

  cv::Mat cardImage = cv::imread(R"(.\resources\cards.png)");
  cv::cvtColor(cardImage, cardImage, cv::COLOR_BGR2GRAY);
  cv::threshold(cardImage, cardImage, kThreshold, 255, cv::THRESH_BINARY);
  // utils::showImage(cardImage, "freecell_card_gray");

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
      std::cout << fmt::format("Matching {}, value {}\n",
                               formatCard(suite, rank), maxValue);
      auto& location = maxLocation;
      cv::Rect headerRect(location + kFirstCardOffset,
                          location + kFirstCardOffset +
                              cv::Point(cardSymbol.cols, cardSymbol.rows));
      cv::rectangle(debugImage, headerRect,
                    utils::kDebugColors[utils::DebugColors::Red], 2);
      cv::putText(debugImage,
                  kRankNames.at(static_cast<Rank>(rank)) +
                      kSuiteSymbols.at(static_cast<Suite>(suite)),
                  location + cv::Point(-40, 10) + kFirstCardOffset,
                  cv::FONT_HERSHEY_SIMPLEX, 0.8,
                  utils::kDebugColors[utils::DebugColors::Yellow], 2);
      cards.push_back(
          {static_cast<Suite>(suite), static_cast<Rank>(rank), location});
      auto [col, row] = getCardColumnAndRow(location);
      if (col < 0 || col >= 8) {
        LOG(ERROR) << fmt::format("Invalid column {} for location ({}, {})",
                                  col, location.x, location.y);
        continue;
      }
      if (row < 0 || (col < 4 && row >= 7) || (col >= 4 && row >= 6)) {
        LOG(ERROR) << fmt::format("Invalid row {} for location ({}, {})", row,
                                  location.x, location.y);
        continue;
      }
      deck_[col][row] = {static_cast<Suite>(suite), static_cast<Rank>(rank)};
    }
  }

  // std::sort(cards.begin(), cards.end(),
  //           [](auto& a, auto& b) { return a.location.x < b.location.x; });
  // int index = 0;
  // for (int col = 0; col < 8; col++) {
  //   int numCards = col < 4 ? 7 : 6;
  //   std::sort(cards.begin() + index, cards.begin() + index + numCards,
  //             [](auto& a, auto& b) { return a.location.y < b.location.y; });
  //   for (size_t i = index; i < index + numCards; i++) {
  //     auto& card = cards.at(i);
  //     auto row = i - index;
  //     std::cout << fmt::format("Col {}, Row {} => Rank {}, Suite {}\n", row +
  //     1,
  //                              col + 1, kRankNames.at(card.rank),
  //                              kSuiteSymbols.at(card.suite));
  //     if (deck_[col][row].rank != Rank::INVALID) {
  //       // duplicate
  //       deck_[col][row].rank = Rank::INVALID;
  //       continue;
  //     }
  //     deck_[col][row] = Card{card.suite, card.rank};
  //   }
  //   index += numCards;
  // }
  // DCHECK_EQ(index, 52);
  printDeck(deck_);
  utils::showImage(debugImage, "freecell_card_locations");
  return true;
}

std::pair<int, int> FreecellRecognizer::getCardColumnAndRow(
    const cv::Point& location) {
  /* int col = (location.x - kFirstCardOffset.x) / kHorizontalDistance;
   int row = (location.y - kFirstCardOffset.y) / kVerticalDistance;*/
  int col = location.x / kHorizontalDistance;
  int row = location.y / kVerticalDistance;
  return {col, row};
}

bool FreecellRecognizer::recognize() { return recognizeTemplateMatching(); }

bool FreecellRecognizer::recognize_sift() {
  auto snapshot = gameWindow_->getSnapshot();
  cv::Mat debugImage = snapshot.clone();
  cv::Mat suitesImage = cv::imread(R"(.\resources\suites.png)");
  // cv::cvtColor(suitesImage, suitesImage, cv::COLOR_BGR2GRAY);

  cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
  std::vector<cv::KeyPoint> keypointsTarget;
  cv::Mat descriptorsTarget;
  sift->detectAndCompute(suitesImage, cv::noArray(), keypointsTarget,
                         descriptorsTarget);
  cv::BFMatcher bfMatcher(cv::NORM_L2);
  cv::namedWindow("DEBUG");
  for (int col = 0; col < 8; col++) {
    int numCards = col < 4 ? 7 : 6;
    for (int row = 0; row < numCards; row++) {
      cv::Rect symbolRect(kFirstCardOffset.x + col * kHorizontalDistance,
                          kFirstCardOffset.y + row * kVerticalDistance,
                          kCardSymbolWidth, kCardSymbolHeight / 2);
      auto cardSymbol = snapshot(symbolRect);
      cv::cvtColor(cardSymbol, cardSymbol, cv::COLOR_BGR2GRAY);
      cv::threshold(cardSymbol, cardSymbol, 140, 255, cv::THRESH_BINARY);
      // recognize rank
      tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
      ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT);
      ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
      ocr->SetVariable("debug_file", "NUL");
      ocr->SetVariable("tessedit_char_whitelist", "0123456789AJQK");
      ocr->SetImage(cardSymbol.data, cardSymbol.cols, cardSymbol.rows, 1,
                    cardSymbol.step);
      auto str = std::string(ocr->GetUTF8Text());
      char rank;
      if (str.empty()) {
        rank = 'X';
      }
      if (str.at(0) == '1') {
        rank = 'T';
      } else {
        rank = str.at(0);
      }

      // recognize suite
      cv::Rect suiteRect(
          kFirstCardOffset.x + col * kHorizontalDistance,  // add some offset?
          kFirstCardOffset.y + row * kVerticalDistance + kCardSymbolHeight / 2,
          kCardSymbolWidth, 18);
      auto cardSuite = snapshot(suiteRect);

      std::vector<cv::KeyPoint> keypointsTemplate;
      cv::Mat descriptorsTemplate;
      sift->detectAndCompute(cardSuite, cv::noArray(), keypointsTemplate,
                             descriptorsTemplate);
      std::vector<std::vector<cv::DMatch>> knnMatches;
      bfMatcher.knnMatch(descriptorsTemplate, descriptorsTarget, knnMatches, 2);
      std::vector<cv::DMatch> goodMatches;
      for (size_t i = 0; i < knnMatches.size(); ++i) {
        if (knnMatches[i][0].distance < 0.75 * knnMatches[i][1].distance) {
          goodMatches.push_back(knnMatches[i][0]);
        }
      }
      cv::Mat matchImage;
      cv::drawMatches(cardSuite, keypointsTemplate, suitesImage,
                      keypointsTarget, goodMatches, matchImage);
      cv::imshow("DEBUG", matchImage);
      cv::waitKey(0);

      std::cout << fmt::format("Processing col {}, row {}, card = {}\n", col,
                               row, rank);
      cv::putText(debugImage, std::string(1, rank),
                  symbolRect.tl() + cv::Point(-40, 20),
                  cv::FONT_HERSHEY_SIMPLEX, 0.8,
                  utils::kDebugColors.at(utils::DebugColors::Yellow), 2);
    }
  }

  utils::showImage(debugImage, "freecell_ocr_results");
  return true;
}

bool FreecellRecognizer::recognizeSuites() {
  std::string suites = "CSDH";
  cv::Mat screenshot = gameWindow_->getSnapshot();
  cv::Mat screenshotGray;
  cv::cvtColor(screenshot, screenshotGray, cv::COLOR_BGR2GRAY);
  // suite image: 1 row 4 col, 32x32 each
  cv::Mat suitesImage =
      cv::imread(R"(.\resources\suites_large.png)", cv::IMREAD_GRAYSCALE);

  auto sift = cv::SIFT::create();
  std::vector<cv::KeyPoint> screenKeypoints;
  cv::Mat screenDescriptors;
  sift->detectAndCompute(screenshotGray, cv::noArray(), screenKeypoints,
                         screenDescriptors);

  std::vector<cv::KeyPoint> suiteKeypoints;
  cv::Mat suiteDescriptors;
  std::vector<std::vector<cv::DMatch>> knnMatches;
  std::vector<cv::DMatch> goodMatches;

  auto bfMatcher = cv::BFMatcher::create();
  for (int i = 0; i < 4; i++) {
    std::cout << "Calculating " << i << "...\n";
    cv::Rect suiteRect(i * 32, 0, 32, 32);
    cv::Mat suiteImage = suitesImage(suiteRect);
    sift->detectAndCompute(suiteImage, cv::noArray(), suiteKeypoints,
                           suiteDescriptors);
    bfMatcher->knnMatch(suiteDescriptors, screenDescriptors, knnMatches, 2);
    for (size_t i = 0; i < knnMatches.size(); i++) {
      if (knnMatches[i][0].distance < 0.75 * knnMatches[i][1].distance) {
        goodMatches.push_back(knnMatches[i][0]);
      }
    }
    std::cout << fmt::format("Found {} good matches\n", goodMatches.size());
    for (const auto& match : goodMatches) {
      auto& point = screenKeypoints[match.trainIdx].pt;
      cv::circle(screenshot, point, 10,
                 utils::kDebugColors[utils::DebugColors::Red], 2);
    }
    cv::imshow("Debug", screenshot);
    cv::waitKey(0);
  }
  return true;
}

void FreecellRecognizer::printCardWithLocation(const CardWithLocation& card) {
  std::cout << kRankNames.at(card.rank) << kSuiteSymbols.at(card.suite) << " ";
}
}  // namespace freecell
}  // namespace game_assistant