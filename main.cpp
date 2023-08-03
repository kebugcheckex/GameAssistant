#include "pch.h"
#include <opencv2/highgui.hpp>

#include "GameWindow.h"
#include "OpenCVPlayground.h"
#include "Player.h"
#include "SudokuRecognizer.h"
#include "SudokuSolver.h"
#include "glog/logging.h"

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

  auto gameMode = GameModeMap.at(FLAGS_game_mode);
  auto gameWindow = std::make_shared<GameWindow>(
      FLAGS_dev_mode ? FLAGS_image_file_path : kGameWindowName.data());
  auto recognizer = std::make_shared<SudokuRecognizer>(gameMode, gameWindow);
  auto result = recognizer->recognize();
  /*auto solver =
      std::make_shared<SudokuSolver>(recognizer->getRecognizedBoard());
  Player player(gameWindow, recognizer, solver, gameMode);
  player.play();*/
  return 0;
}
