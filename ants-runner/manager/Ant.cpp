#include "Ant.hpp"

MetaAnt::MetaAnt(int teamId, antgui::Point startPos)
  : teamId(teamId), pos(startPos)
{
    ftime = 0;
    frozen = false;
    withFood = false;
}

bool MetaAnt::isFrozen() const
{
    return frozen;
}

antgui::Point MetaAnt::getPoint() const
{
    return pos;
}

char* MetaAnt::getMemory() const
{
    return const_cast<char *>(mem);
}

bool MetaAnt::hasFood() const
{
    return withFood;
}

int MetaAnt::getTeamId() const
{
    return teamId;
}
