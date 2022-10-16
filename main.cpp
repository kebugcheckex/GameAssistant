#include "pch.h"

#include <opencv2/highgui.hpp>

#include "SudokuRecognizer.h"
#include "SudokuSolver.h"
#include "GameWindow.h"
#include "Player.h"
#include "glog/logging.h"

DEFINE_bool(automatic, false, "Play the game automatically");
DEFINE_bool(debug, false, "Debug mode, show intermediate step data");

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

int main()
{
    init_apartment();
    
    SudokuRecognizer recognizer;
    GameWindow gameWindow;

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
    auto windowRect = gameWindow.getWindowRect();
    auto boardRect = recognizer.getBoardRect();
    boardRect.left += windowRect.left;
    boardRect.top += windowRect.top;
    boardRect.right += windowRect.left;
    boardRect.bottom += windowRect.top;
    
    if (FLAGS_automatic) {
      Player player(gameWindow.getMonitorRect());
      player.play(boardRect, solver.getSolvedBoard());
    }
    return 0;
}
