#include "AntManager.hpp"
#include <memory.h>
#include <algorithm>

using std::pair;

#define FREEZE_TIME 8

MetaAnt::MetaAnt(int teamId, antgui::Point startPos)
  : teamId(teamId), pos(startPos), mem(new char[antlogic::MAX_MEMORY]())
{
}

bool MetaAnt::isFrozen() const
{
    return ftime;
}

void MetaAnt::tick()
{
	if (ftime)
		--ftime;
}

void Cell::tick()
{
	if (smellIntensity)
		if (!--smellIntensity)
			smell = 0;
}

antgui::Point MetaAnt::getPoint() const
{
    return pos;
}

char* MetaAnt::getMemory() const
{
    return mem.get();
}

bool MetaAnt::hasFood() const
{
    return withFood;
}

int MetaAnt::getTeamId() const
{
    return teamId;
}

antlogic::AntSensor Cell::sense(Team* team) const
{
    antlogic::AntSensor res;

    res.isEnemy = false;
    res.isFriend = false;

    res.isEnemyHill = false;
    res.isMyHill = false;

    res.isFood = food;
    res.isWall = isWall;
	
	for (auto& i : ants)
	{
		if (i->teamId == team->id)
		{
			(i->teamId == team->id ? res.isFriend : res.isEnemy) = true;
        }
    }

    if (team->id == hillId)
    {
        res.isMyHill = true;
    }
    else if (hillId != -1)
    {
        res.isEnemyHill = true;
    }

    res.smell = smell;
    res.smellIntensity = smellIntensity;

    return res;
}

//
//  AntManager
//

Point AntManager::hillOf (int teamId)
{
	Point res;
	res.x = 1 + (width - 1) * (teamId % 2);
	res.y = 1 + (height - 1) * ((teamId % 4) > 1);
	return res;
}

AntManager::AntManager(const GameSettings& _settings, int _height, int _width, int _maxAntCountPerTeam):
	settings(_settings), height(_height), width(_width), curStep(0),
	maxAntCountPerTeam(_maxAntCountPerTeam)
{
	for (int x = 0; x <= width + 1; x++)
	{
		for (int y = 0; y <= height + 1; y++)
		{
			mainField.insert(pair<Point, Cell>(Point(x, y), Cell()));
		}
	}

	// walls
	Point left(0, 0);
	Point right(width + 1, 0);
	for (int y = 0; y <= height + 1;y++)
	{
		left.y = y;
		right.y = y;
		mainField[left].isWall = true;
		mainField[right].isWall = true;
	}
	Point top(0, 0);
	Point bottom(0, height + 1);
	for (int x = 0; x <= width + 1; x++)
	{
		top.x = x;
		bottom.x = x;
		mainField[top].isWall = true;
		mainField[bottom].isWall = true;
	}

	int tid = 0;
	for (auto i : settings.players)
	{
		Team *t;
		teams.emplace_back(t = new Team(tid));
		t->ai = antlogic::IAntLogic::GetAntLogic(i, string("w") + to_string(tid));
		mainField[t->hill = hillOf(tid)].hillId = tid;
		++tid;
	}
}

void AntManager::step()
{
	if (!curStep)
	{
		ifstream foodStream(settings.mapName);
		int x, y, f;
		while (foodStream >> x >> y >> f)
		{
			mainField[Point(x, y)].food = f;
			gui->SetFood(ConcreteFood(Point(x, y), f));
		}
	}
	if (curStep < maxAntCountPerTeam)
	{
		/* ants */
		for (auto& i : teams)
		{
			i->ants.emplace_back(new MetaAnt(i->id, i->hill));
		}
	}
	
	for (auto& i : mainField)
	{
		i.second.tick();
	}

	for (auto& team : teams)
	{
		for (auto& ant : team->ants)
		{
			ant->tick();
			if (ant->isFrozen()) continue;
			antlogic::AntSensor sensors[3][3];
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					Point p = ant->pos; p.x += i; p.y += j;
					sensors[i + 1][j + 1] = mainField[p].sense(&*team);
				}
			}
			antlogic::AntAction res = team->ai->GetAction(*ant, sensors);
			switch (res.actionType)
			{
				case antlogic::AntActionType::MOVE_UP:
					move(&*ant, 0, -1);
				break;
				case antlogic::AntActionType::MOVE_DOWN:
					move(&*ant, 0, 1);
				break;
				case antlogic::AntActionType::MOVE_LEFT:
					move(&*ant, -1, 0);
				break;
				case antlogic::AntActionType::MOVE_RIGHT:
					move(&*ant, 1, 0);
				break;
				case antlogic::AntActionType::BITE_UP:
					bite(&*ant, 0, -1);
				break;
				case antlogic::AntActionType::BITE_DOWN:
					bite(&*ant, 0, 1);
				break;
				case antlogic::AntActionType::BITE_LEFT:
					bite(&*ant, -1, 0);
				break;
				case antlogic::AntActionType::BITE_RIGHT:
					bite(&*ant, 1, 0);
				break;
				case antlogic::AntActionType::GET:
					food(&*ant, false);
				break;
				case antlogic::AntActionType::PUT:
					food(&*ant, true);
				break;
				default:
				break;
			}
		}
	}
	gui->BeginPaint();
	for (auto& i : teams)
	{
		gui->SetTeamScore(i->id, mainField[i->hill].food);
		for (auto& j : i->ants)
		{
			gui->SetAnt(*j);
		}
	}
	gui->EndPaint();
	
	++curStep;
}

void AntManager::move(MetaAnt* ant, int dx, int dy)
{
	Point newPoint = ant->pos;
	newPoint.x += dx;
	newPoint.y += dy;
	if (mainField[newPoint].isWall)
	{
		return;
	}
	mainField[ant->pos].ants.erase(ant);
	ant->pos = newPoint;
	mainField[ant->pos].ants.insert(ant);
}

void AntManager::bite(MetaAnt* ant, int dx, int dy)
{
	Point bitePoint = ant->pos;
	bitePoint.x += dx;
	bitePoint.y += dy;
	if (mainField[bitePoint].ants.empty())
	{
		return;
	}
	vector<MetaAnt*> candidates;
	for (auto& i : mainField[bitePoint].ants)
	{
		candidates.push_back(i);
	}
	sort(candidates.begin(), candidates.end(), [](MetaAnt* a, MetaAnt* b) {
		return a->ftime < b->ftime;
	});
	auto end = candidates.begin() + 1;
	while (end != candidates.end())
	{
		if ((*end)->ftime != (*(end - 1))->ftime) break;
		++end;
	}
	candidates[(rand() % (end - candidates.begin()))]->ftime = FREEZE_TIME;
}

void AntManager::food(MetaAnt* ant, bool put)
{
	auto &cell = mainField[ant->pos];
	if (!put && cell.food && !ant->withFood)
	{
		--cell.food;
		ant->withFood = true;
	}
	else if (put && ant->withFood)
	{
		++cell.food;
		ant->withFood = false;
	}
	gui->SetFood(ConcreteFood(ant->pos, cell.food));
}

void AntManager::setGui(std::shared_ptr<antgui::IAntGui> gui)
{
	this->gui = gui;
}
