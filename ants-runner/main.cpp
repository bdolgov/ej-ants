#include "main_window.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "manager/AntManager.hpp"
#include <fstream>

int main(int argc, char *argv[])
{
    GameSettings settings;
    cin >> settings.mapName;
    cin >> settings.stepLimit;
    cin >> settings.timeLimit;

    std::string pl;
    
    while (cin >> pl) {
        settings.players.push_back(pl);
    }
    if (argc >= 3 && string(argv[1]) == "-t")
    {
        settings.dumpScore = string(argv[2]);
    }
    main_window win(settings);
    for (int i = 0; i <= settings.stepLimit; ++i) {
        win.nextStep();
    }
    if (settings.dumpScore != "")
    {
        ofstream f(settings.dumpScore);
        f << win.manager.score(0) << " " << win.manager.score(1) << " " << win.manager.score(2) << " " << win.manager.score(3) << endl;
    }

    return 0;
}
