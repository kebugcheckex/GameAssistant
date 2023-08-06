#include "pch.h"


#include "GameWindow.h"
#include "OpenCVPlayground.h"
#include "Player.h"
#include "SudokuBoard.h"
#include "SudokuRecognizer.h"

static std::unordered_map<std::string, GameMode> const GameModeMap = {
    {"classic", GameMode::CLASSIC},
    {"irregular", GameMode::IRREGULAR},
    {"icebreaker", GameMode::ICE_BREAKER},
};

static bool validateGameMode(const char* flagName, const std::string& value) {
  if (GameModeMap.find(value) == GameModeMap.end()) {
    LOG(ERROR) << fmt::format("Invalid value for option --{} {}", flagName,
                              value);
    return false;
  }
  return true;
}

DEFINE_bool(debug, false, "Debug mode, show intermediate step data");
DEFINE_bool(multirun, false, "Do not exit after finishing one run");
DEFINE_string(image_file, "", "Load an image instead of taking a screenshot from the game window");
DEFINE_string(game_mode, "classic,irregular,icebreaker", "Game mode");
DEFINE_validator(game_mode, &validateGameMode);

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  init_apartment();
  
  auto gameMode = GameModeMap.at(FLAGS_game_mode);
  auto gameWindow =
      std::make_shared<GameWindow>(FLAGS_image_file != "" ? FLAGS_image_file
                                                 : kGameWindowName.data());
  auto recognizer = std::make_shared<SudokuRecognizer>(gameMode, gameWindow);
  if (!recognizer->recognize()) {
    LOG(ERROR) << "failed to recognize board";
    return 0;
  }
  auto sudokuBoard = std::make_shared<SudokuBoard>(
      recognizer->getRecognizedBoard(), recognizer->getBlocks());
  Player player(gameWindow, recognizer, sudokuBoard, gameMode);
  player.play();
  return 0;
}
