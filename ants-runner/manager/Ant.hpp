#ifndef ANT_HPP
#define ANT_HPP

#include "IAntGUI.cpp"
#include "IAntLogic.hpp"

class MetaAnt : public antgui::Ant, public antlogic::Ant
{
    int           teamId;
    antgui::Point pos;
    char          mem[antlogic::MAX_MEMORY];
    bool          frozen;
    int           ftime;
    bool          withFood;

public:
    MetaAnt(int teamId, antgui::Point startPos);

    // gui needs following
    bool isFrozen() const;
    antgui::Point getPoint() const;

    // logic needs following
    char* getMemory() const;

    // both need following
    bool hasFood() const;
    int getTeamId() const;

    friend class AntManager;
};

#endif // ANT_HPP
