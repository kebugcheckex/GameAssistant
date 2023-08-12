#include "pch.h"

#include "FreecellRecognizer.h"
#include "GameWindow.h"
#include "Player.h"
#include "RecognizerUtils.h"
#include "SudokuBoard.h"
#include "SudokuData.h"
#include "SudokuRecognizer.h"

using namespace game_assistant;

// TODO move this into some sudoku class
static std::unordered_map<std::string, sudoku::GameMode> const GameModeMap = {
    {"classic", sudoku::GameMode::CLASSIC},
    {"irregular", sudoku::GameMode::IRREGULAR},
    {"icebreaker", sudoku::GameMode::ICE_BREAKER},
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
DEFINE_string(
    image_file, "",
    "Load an image instead of taking a screenshot from the game window");
DEFINE_string(game, "", "Which game to play: sudoku, freecell, utils");
DEFINE_string(game_mode, "classic,irregular,icebreaker", "Game mode");
// DEFINE_validator(game_mode, &validateGameMode);

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  init_apartment();

  if (FLAGS_game == "sudoku") {
    auto gameMode = GameModeMap.at(FLAGS_game_mode);
    auto gameWindow = std::make_shared<GameWindow>(
        FLAGS_image_file != "" ? FLAGS_image_file : kGameWindowName.data());
    auto recognizer =
        std::make_shared<sudoku::SudokuRecognizer>(gameMode, gameWindow);
    if (!recognizer->recognize()) {
      LOG(ERROR) << "failed to recognize board";
      return 0;
    }
    auto sudokuBoard = std::make_shared<sudoku::SudokuBoard>(
        recognizer->getRecognizedBoard(), recognizer->getBlocks());
    Player player(gameWindow, recognizer, sudokuBoard, gameMode);
    player.play();
  } else if (FLAGS_game == "freecell") {
    auto gameWindow = std::make_shared<GameWindow>(
        FLAGS_image_file != "" ? FLAGS_image_file : kSolitareWindowName.data());
    game_assistant::freecell::FreecellRecognizer recognizer(gameWindow);
    recognizer.recognize();
  } else if (FLAGS_game == "utils") {
    freecell::generateFreecellTemplate(FLAGS_image_file);
  } else {
    std::cerr << fmt::format("Unknown game {}\n", FLAGS_game);
    return 1;
  }
  return 0;
}
