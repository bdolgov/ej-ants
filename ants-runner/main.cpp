#include "main_window.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "manager/AntManager.hpp"

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

    main_window win(settings);
    for (int i = 0; i <= settings.stepLimit; ++i) {
        win.nextStep();
    }

    return 0;
}
