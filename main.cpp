#include "pch.h"

#include <opencv2/highgui.hpp>

#include "GameWindow.h"
#include "Player.h"
#include "SudokuRecognizer.h"
#include "SudokuSolver.h"
#include "glog/logging.h"
#include "OpenCVPlayground.h"

static std::unordered_map<std::string, GameMode> const GameModeMap = {
    {"classic", GameMode::CLASSIC},
    {"irregular", GameMode::IRREGULAR},
    {"icebreaker", GameMode::ICE_BREAKER},
};

static bool validateGameMode(const char* flagName, const std::string& value) {
  if (GameModeMap.find(value) == GameModeMap.end()) {
    std::cerr << "Invalid value for option --" << flagName << " " << value
              << "\n";
    return false;
  }
  return true;
}

DEFINE_bool(automatic, false, "Play the game automatically");
DEFINE_bool(debug, false, "Debug mode, show intermediate step data");
DEFINE_bool(multirun, false, "Do not exit after finishing one run");
DEFINE_bool(dev_mode, false, "Load file from image instead of screenshot");
DEFINE_string(image_file_path, "", "Image for dev mode");
DEFINE_string(game_mode, "classic,irregular,icebreaker", "Game mode");
DEFINE_validator(game_mode, &validateGameMode);

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

int main(int argc, char* argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  init_apartment();

  auto gameMode = GameModeMap.at(FLAGS_game_mode);;
  auto gameWindow = std::make_shared<GameWindow>(
      FLAGS_dev_mode ? FLAGS_image_file_path : kGameWindowName.data());
  SudokuRecognizer recognizer(gameMode, gameWindow);
  if (!recognizer.recognize()) {
    LOG(ERROR) << "Failed to recognize borard";
    return 1;
  }
  auto iceBoard = recognizer.getIceBoard();

  SudokuSolver solver(recognizer.getRecognizedBoard());
  solver.solve();
  auto board = solver.getSolvedBoard();
  Player::playIceBreaker(board, iceBoard);

  if (FLAGS_automatic) {
    auto windowRect = gameWindow->getWindowRect();
    auto boardRect = recognizer.getBoardRect();
    boardRect.left += windowRect.left;
    boardRect.top += windowRect.top;
    boardRect.right += windowRect.left;
    boardRect.bottom += windowRect.top;
    Player player(gameWindow->getMonitorRect(), boardRect);
    player.playIceBreaker(solver.getSolvedBoard(), iceBoard);
  }
  return 0;
}
