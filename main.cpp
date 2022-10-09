#include "pch.h"

#include <opencv2/highgui.hpp>

#include "SudokuRecognizer.h"
#include "SudokuSolver.h"
#include "GameWindow.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

int main(int argc, char *argv[])
{
    init_apartment();
    
    SudokuRecognizer recognizer;
    GameWindow gameWindow("test");
    auto image = gameWindow.getSnapshot();
    recognizer.loadImage(image);
    recognizer.recognize();
    SudokuSolver solver(recognizer.getResults());
    solver.solve();
    solver.printResults();
    return 0;
}
