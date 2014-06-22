#include "main_window.hpp"
#include <memory>

enum {
    PLAYER_COUNT = 4,
    FIELD_SIZE = 31,
    ANT_COUNT = 10
};

main_window::main_window(const GameSettings &settings)
  : manager(settings, FIELD_SIZE, FIELD_SIZE, ANT_COUNT)
{
    this->step = 0;
    using namespace antgui;
    gui = std::dynamic_pointer_cast<AntGuiLogImpl>(IAntGui::getGUI());
    if (gui)
    {
        gui->init();
    }
    manager.setGui(gui);
    //manager.setFoodGeneretor(make_shared<food_iterator>(FIELD_SIZE));
    //manager.setFoodGeneretor(make_shared<file_food_iterator>(settings.mapName));
}

void main_window::nextStep(void)
{
    manager.step();
    gui->Paint();
}
