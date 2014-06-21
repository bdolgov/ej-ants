#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "AntManager.hpp"
#include "IAntGuiImpl.hpp"

#include <memory>

class main_window
{
public:
    AntManager manager;
    main_window(const GameSettings& );
    void nextStep(void);
private:
    int step;
    std::shared_ptr<antgui::AntGuiLogImpl> gui;
};


#endif //MAIN_WINDOW_HPP
