#include "pch.h"

#include <opencv2/highgui.hpp>

#include "GameWindow.h"
#include "Player.h"
#include "SudokuRecognizer.h"
#include "SudokuSolver.h"
#include "glog/logging.h"

DEFINE_bool(automatic, false, "Play the game automatically");
DEFINE_bool(debug, false, "Debug mode, show intermediate step data");
DEFINE_bool(multirun, false, "Do not exit after finishing one run");
DEFINE_bool(dev_mode, false, "Load file from image instead of screenshot");
DEFINE_string(image_file_path, "", "Image for dev mode");
DEFINE_string(game_mode, "classic,irregular,icebreaker", "Game mode");

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

constexpr std::string_view kCvWindowName{"Auto Sudoku"};

typedef struct tagRecognizeButtonData {
  std::shared_ptr<SudokuRecognizer> recognizer;
  std::shared_ptr<GameWindow> gameWindow;
} RecognizeButtonData;

void handleRecognizeButtonClick(int state, void* userData) {
  try {
    auto data = (RecognizeButtonData*)userData;
    auto image = data->gameWindow->getSnapshot();
    data->recognizer->loadImage(image);
    data->recognizer->recognize();
  } catch (std::exception& ex) {
    LOG(ERROR) << ex.what();
  }
}

void handleAutoPlayButtonClick(int state, void* userData) {}

int main(int argc, char* argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  init_apartment();

  auto recognizer = std::make_shared<SudokuRecognizer>();
  auto gameWindow =std::make_shared < GameWindow>(FLAGS_dev_mode ? FLAGS_image_file_path
                                       : kGameWindowName.data());
  RecognizeButtonData* recognizeButtonData = nullptr;
  recognizeButtonData->recognizer = recognizer;
  recognizeButtonData->gameWindow = gameWindow;
  cv::namedWindow(kCvWindowName.data());
  cv::createButton("Recognize", handleRecognizeButtonClick, (void *)recognizeButtonData);
  cv::createButton("Auto Play", handleAutoPlayButtonClick);

  SudokuSolver solver(recognizer->getResults());
  solver.solve();
  auto board = solver.getResults();

    //auto windowRect = gameWindow->getWindowRect();
    //auto boardRect = recognizer->getBoardRect();
    //boardRect.left += windowRect.left;
    //boardRect.top += windowRect.top;
    //boardRect.right += windowRect.left;
    //boardRect.bottom += windowRect.top;
    //Player player(gameWindow->getMonitorRect(), boardRect);
    //player.play(solver.getSolvedBoard());
 
  return 0;
}
