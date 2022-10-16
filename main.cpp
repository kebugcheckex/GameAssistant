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

int main(int argc, char* argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  init_apartment();

  SudokuRecognizer recognizer;
  GameWindow gameWindow(FLAGS_dev_mode ? FLAGS_image_file_path
                                       : kWindowName.data());

  while (true) {
    try {
      auto image = gameWindow.getSnapshot();
      recognizer.loadImage(image);
      recognizer.recognize();
      break;
    } catch (std::exception& ex) {
      LOG(ERROR) << ex.what() << " Sleep for 3 seconds and try again";
      Sleep(3000);
    }
  }

  SudokuSolver solver(recognizer.getResults());
  solver.solve();
  auto board = solver.getResults();

  if (FLAGS_automatic) {
    auto windowRect = gameWindow.getWindowRect();
    auto boardRect = recognizer.getBoardRect();
    boardRect.left += windowRect.left;
    boardRect.top += windowRect.top;
    boardRect.right += windowRect.left;
    boardRect.bottom += windowRect.top;
    Player player(gameWindow.getMonitorRect(), boardRect);
    player.play(solver.getSolvedBoard());
  }
  return 0;
}
