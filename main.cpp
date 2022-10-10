#include "pch.h"

#include <opencv2/highgui.hpp>

#include "SudokuRecognizer.h"
#include "SudokuSolver.h"
#include "GameWindow.h"
#include "Player.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

int main()
{
    init_apartment();
    
    SudokuRecognizer recognizer;
    GameWindow gameWindow;
    auto image = gameWindow.getSnapshot();
    recognizer.loadImage(image);
    recognizer.recognize();
    SudokuSolver solver(recognizer.getResults());
    solver.solve();

    auto windowRect = gameWindow.getWindowRect();
    auto boardRect = recognizer.getBoardRect();
    boardRect.left += windowRect.left;
    boardRect.top += windowRect.top;
    boardRect.right += windowRect.left;
    boardRect.bottom += windowRect.top;
    Player player(gameWindow.getMonitorRect());
    player.play(boardRect, solver.getSolvedBoard());
    return 0;
}
