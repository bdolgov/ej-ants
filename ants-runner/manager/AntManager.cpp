#include "AntManager.hpp"
#include <memory.h>

using std::pair;

bool operator!=(const food_iterator& it1, const food_iterator& it2)
{
    return it1.iteration != it2.iteration;
}

void food_iterator0::WakeUp()
{
    while (!s.empty() && s.top().n > 1)
    {
        FuncParam param = s.top();
        if (param.n > 1)
        {
            s.pop();
            for (int i = 0; i < 5; i++)
            {
                Point p(param.p.x - dx[i] * param.fsize / fractal_den, param.p.y + dy[i] * param.fsize / fractal_den);
                FuncParam new_param(param.n - 1, p, param.count + df[i], param.fsize / fractal_den);
                if (new_param.n > 1 || (p.x >= 0 && p.y >= 0 && p.x < size && p.y < size))
                {
                    s.push(new_param);
                }
            }
        }
    }
}

food_iterator0::food_iterator0(int size) : size(size)
{
    int t = 1;
    int n = 1;
    while (t < size)
    {
        t *= fractal_den;
        n++;
    }
    s.push(FuncParam(n, Point(size / 2, size / 2), 40, t));
}

ConcreteFood food_iterator0::operator*()
{
    WakeUp();
    return ConcreteFood(s.top().p, s.top().count);
}

food_iterator0& food_iterator0::operator++()
{
    this->operator *();
    s.pop();
    WakeUp();
    if (s.empty())
    {
        iteration = -1;
    }
    return *this;
}

food_iterator0 food_iterator0::operator++(int)
{
    this->operator *();
    food_iterator tmp = *this;
    ++*this;
    return tmp;
}

food_iterator0::food_iterator0(const food_iterator0& it) : size(it.size), iteration(it.iteration)
{
    if (!it.s.empty())
    {
        s.push(it.s.top());
    }
}

MetaAnt::MetaAnt(int teamId, antgui::Point startPos)
  : teamId(teamId), pos(startPos)
{
    ftime = 0;
    frozen = false;
    withFood = false;
    for (int i = 0; i < antlogic::MAX_MEMORY; i++)
    {
        mem[i] = 0;
    }
}

bool MetaAnt::isFrozen() const
{
    return frozen;
}

antgui::Point MetaAnt::getPoint() const
{
    Point resPos;
    resPos.x = pos.x;
    resPos.y = pos.y;
    return resPos;
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

antlogic::AntSensor Cell::sense(int teamId) const
{
    antlogic::AntSensor res;

    res.isEnemy = false;
    res.isFriend = false;

    res.isEnemyHill = false;
    res.isMyHill = false;

    res.isFood = isFood;
    res.isWall = isWall;

    if (isAnt)
    {
        for (unsigned int i = 0; i < ants.size(); i++)
        {
            if (ants[i]->teamId == teamId)
            {
                res.isFriend = true;
            }
            else
            {
                res.isEnemy = true;
            }
        }
    }

    if (isHill)
    {
        if (teamId == hillId)
        {
            res.isMyHill = true;
        }
        else
        {
            res.isEnemyHill = true;
        }
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

AntManager::AntManager(const GameSettings& settings, int height, int width, int maxAntCountPerTeam)
{
    this->settings = settings;
    this->height = height;
    this->width = width;
    this->teamCount = settings.players.size();
    this->maxAntCountPerTeam = maxAntCountPerTeam;

	// main field

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

	// camps

	for (int i = 0; i < teamCount; i++)
	{
		mainField[hillOf(i)].isHill = true;
		mainField[hillOf(i)].hillId = i;
	}

	// ants

	teams.resize(settings.players.size());

	for (int i = 0; i < teamCount; i++)
	{
		teams[i].ants.clear();
        std::string id = "w";
        id += '0' + i;
		ai.push_back(antlogic::IAntLogic::GetAntLogic(settings.players[i], id));
	}

//	printf("AntManager initialized...\n\n");
}

void AntManager::step(int iRun)
{
    //printf("\nTurn #%d:\n", iRun);

    if (iRun == 0)
    {
        food_iterator eof;
        while (*warehouse != eof)
        {
            Point foodPos = (**warehouse).getPoint();
            foodPos.x++;
            foodPos.y++;
            int foodCount = (**warehouse).getCount();
            mainField[foodPos].food += foodCount;
            mainField[foodPos].isFood = true;
            (*warehouse)++;
        }
    }

    for (int i = 0; i < teamCount; i++)
    {
        teams[i].ants.reserve(maxAntCountPerTeam * 2);
    }

	if (teams[0].ants.size() < maxAntCountPerTeam)
	{
		for (int i = 0; i < teamCount; i++)
		{
			teams[i].ants.push_back(MetaAnt(i, hillOf(i)));
            mainField[hillOf(i)].ants.push_back(&teams[i].ants[teams[i].ants.size() - 1]);
		}
	}

	/*
	printf("First hill contains ants:\n");
	for (unsigned int k = 0; k < mainField[hillOf(0)].ants.size(); k++)
	{
		printf("%d(team #%d);\n",
			   int(mainField[hillOf(0)].ants[k]), mainField[hillOf(0)].ants[k]->teamId);
	}
	printf("\n");

	printf("Second hill contains ants:\n");
	for (unsigned int k = 0; k < mainField[hillOf(1)].ants.size(); k++)
	{
		printf("%d(team #%d);\n",
			   int(mainField[hillOf(1)].ants[k]), mainField[hillOf(1)].ants[k]->teamId);
	}
	printf("\n");
	getchar();
	*/

    // smell reduction

    for (map<Point, Cell>::iterator it = mainField.begin(); it != mainField.end(); it++)
    {
        if (it->second.smellIntensity > 0)
        {
            it->second.smellIntensity--;
        }
    }

    // actions

    for (unsigned int i = 0; i < teams.size(); i++)
    {
        for (unsigned int j = 0; j < teams[i].ants.size(); j++)
        {
            MetaAnt *player = &teams[i].ants[j];

            if (player->frozen)
            {
                player->ftime--;
                if (player->ftime == 0)
                {
                    player->frozen = false;
                }
                continue;
            }

            // info

            /*
            printf("\nplayer pos: %d, %d\n", player->pos.y, player->pos.x);
            printf("player address: %d\n", int(player));
            printf("that cell contains %d ants:\n", mainField[player->pos].ants.size());
            for (unsigned int k = 0; k < mainField[player->pos].ants.size(); k++)
            {
                printf("%d(team #%d);\n",
                       int(mainField[player->pos].ants[k]), mainField[player->pos].ants[k]->teamId);
            }
            printf("\n");
            */

            antlogic::AntSensor surrounding[3][3];
            char _cell[3][3];

            for (int dx = -1; dx < 2; dx++)
            {
                for (int dy = -1; dy < 2; dy++)
                {
                    Point surPos = player->pos;
                    surPos.x += dx;
                    surPos.y += dy;
                    surrounding[1+dx][1+dy] = mainField[surPos].sense(player->teamId);
                    _cell[1+dx][1+dy] = 178;
                    if (surrounding[1+dx][1+dy].isWall) _cell[1+dx][1+dy] = 'W';
                    if (surrounding[1+dx][1+dy].isFood) _cell[1+dx][1+dy] = 'F';
                    if (mainField[surPos].isAnt)        _cell[1+dx][1+dy] = 'A';
                }
            }

            /*
            printf("pos: (%d,%d)\n", player->getPoint().x, player->getPoint().y);
            printf("%c%c%c  ", _cell[0][0], _cell[1][0], _cell[2][0]);
            printf("%2d %2d %2d\n", surrounding[0][0].smellIntensity, surrounding[1][0].smellIntensity, surrounding[2][0].smellIntensity);
            printf("%c%c%c  ", _cell[0][1], _cell[1][1], _cell[2][1]);
            printf("%2d %2d %2d\n", surrounding[0][1].smellIntensity, surrounding[1][1].smellIntensity, surrounding[2][1].smellIntensity);
            printf("%c%c%c  ", _cell[0][2], _cell[1][2], _cell[2][2]);
            printf("%2d %2d %2d\n", surrounding[0][2].smellIntensity, surrounding[1][2].smellIntensity, surrounding[2][2].smellIntensity);
            getchar();
            */

            antlogic::AntAction newAction = ai[i]->GetAction(*player, surrounding);

            // smell action

            if (newAction.putSmell)
            {
                mainField[player->pos].smell = newAction.smell;
                mainField[player->pos].smellIntensity = 100;
            }

            // main action

            if (newAction.actionType == antlogic::MOVE_UP
              ||newAction.actionType == antlogic::MOVE_DOWN
              ||newAction.actionType == antlogic::MOVE_RIGHT
              ||newAction.actionType == antlogic::MOVE_LEFT)
            {
                Point oldPos = player->pos;
                Point newPos = player->pos;

                // choosing direction

                if (true)
                {
                    if (newAction.actionType == antlogic::MOVE_LEFT)
                    {
                        //printf("LEFT\n");
                        if (newPos.x == 1)
                        {
                            //printf("WARNING: tried to go up to the wall. Ignored.\n");
                            continue;
                        }
                        else
                        {
                            newPos.x--;
                        }
                    }

                    if (newAction.actionType == antlogic::MOVE_RIGHT)
                    {
                        //printf("RIGHT\n");
                        if (newPos.x == width)
                        {
                            //printf("WARNING: tried to go down to the wall. Ignored.\n");
                            continue;
                        }
                        else
                        {
                            newPos.x++;
                        }
                    }

                    if (newAction.actionType == antlogic::MOVE_UP)
                    {
                        //printf("UP\n");

                        if (newPos.y == 1)
                        {
                            //printf("WARNING: tried to go left to the wall. Ignored.\n");
                            continue;
                        }
                        else
                        {
                            newPos.y--;
                        }
                    }

                    if (newAction.actionType == antlogic::MOVE_DOWN)
                    {
                        //printf("DOWN\n");

                        if (newPos.y == height)
                        {
                            //printf("WARNING: tried to go right to the wall. Ignored.\n");
                            continue;
                        }
                        else
                        {
                            newPos.y++;
                        }
                    }
                }

                // moving

                player->pos = newPos;

                Cell& oldCell = mainField[oldPos];
                Cell& newCell = mainField[newPos];

                unsigned int index = 0;
                for (; index < oldCell.ants.size(); index++)
                {
                    if (oldCell.ants[index] == player)
                    {
                        break;
                    }
                }
                if (index >= oldCell.ants.size())
                {
                    //printf("ERROR: didn't find the ant %d in old cell list\n", int(player));
                    //getchar();
                    //exit(0);
                    continue;
                }
                oldCell.ants.erase(oldCell.ants.begin() + index);

                if (oldCell.ants.empty())
                {
                    oldCell.isAnt = false;
                }

                newCell.ants.push_back(player);
                newCell.isAnt = true;

                //printf("new player pos: %d, %d\n", player->pos.y, player->pos.x);
            }
            else if (newAction.actionType == antlogic::BITE_UP
                   ||newAction.actionType == antlogic::BITE_DOWN
                   ||newAction.actionType == antlogic::BITE_LEFT
                   ||newAction.actionType == antlogic::BITE_RIGHT)
            {
                //printf("Bite action is coming...\n");

                Point targetPos = player->pos;
                if (newAction.actionType == antlogic::BITE_LEFT)
                {
                    if (targetPos.x == 1)
                    {
                        //printf("WARNING: tried to bite the wall from right. Ignored.\n");
                        continue;
                    }
                    else
                    {
                        targetPos.x--;
                    }
                }
                if (newAction.actionType == antlogic::BITE_RIGHT)
                {
                    if (targetPos.x == width)
                    {
                        //printf("WARNING: tried to bite the wall from left. Ignored.\n");
                        continue;
                    }
                    else
                    {
                        targetPos.x++;
                    }
                }
                if (newAction.actionType == antlogic::BITE_UP)
                {
                    if (targetPos.y == 1)
                    {
                        //printf("WARNING: tried to bite the wall from down. Ignored.\n");
                        continue;
                    }
                    else
                    {
                        targetPos.y--;
                    }
                }
                if (newAction.actionType == antlogic::BITE_DOWN)
                {
                    if (targetPos.y == height)
                    {
                        //printf("WARNING: tried to bite the wall from up. Ignored.\n");
                        continue;
                    }
                    else
                    {
                        targetPos.y++;
                    }
                }

                Cell& targetCell = mainField[targetPos];

                antlogic::AntSensor targetCellInfo = targetCell.sense(player->teamId);
                if (!targetCellInfo.isEnemy)
                {
                    //printf("WARNING: There is no one to bite. Ignored.\n");
                    continue;
                }

                unsigned int index = 0;
                for (; index < targetCell.ants.size(); index++)
                {
                    if (targetCell.ants[index]->teamId != player->teamId)
                    {
                        break;
                    }
                }
                if (index >= targetCell.ants.size())
                {
//                    printf("ERROR: didn't find the enemy ant");
                    getchar();
                    exit(0);
                }
                MetaAnt* target = targetCell.ants[index];

                target->ftime = 8;
                target->frozen = true;

                if (target->withFood)
                {
                    target->withFood = false;
                    targetCell.food++;
                    targetCell.isFood = true;
                }

                //printf("ant on (%d,%d) was bitten\n", targetPos.x, targetPos.y);
            }
            else if (newAction.actionType == antlogic::GET)
            {
                //printf("GET\n");

                if (player->withFood)
                {
//                    printf("WARNING: Already has food. Ignored.\n");
                    continue;
                }
                Cell& targetCell = mainField[player->pos];
                if (targetCell.food > 0)
                {
                    targetCell.food--;
                    player->withFood = true;
                }

                if (targetCell.food == 0)
                {
                    targetCell.isFood = false;
                }

                if (hillOf(player->teamId).x != player->pos.x
                 || hillOf(player->teamId).y != player->pos.y)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (hillOf(i).x == player->pos.x
                        && hillOf(i).y == player->pos.y) {
                            if (teams[i].score > 0) {
                                 teams[i].score -= 1;
                            }
                        }
                    }
                }
            }
            else if (newAction.actionType == antlogic::PUT)
            {
                //printf("PUT\n");

                if (!player->withFood)
                {
//                    printf("WARNING: Has no food to put. Ignored.\n");
                    continue;
                }
                Cell& targetCell = mainField[player->pos];
                targetCell.food++;
                targetCell.isFood = true;
                player->withFood = false;

                if (hillOf(player->teamId).x == player->pos.x
                 && hillOf(player->teamId).y == player->pos.y)
                {
                    teams[player->teamId].score += 1;
                }
            }
            else
            {
//                printf("WARNING: Incorrect Action. Ignored.\n");
            }
        }
    }

    // gui painting

    gui->BeginPaint();
    for (unsigned int i = 0; i < teams.size(); i++)
    {
        for (unsigned int j = 0; j < teams[i].ants.size(); j++)
        {
            gui->SetAnt(teams[i].ants[j]);
        }

        for (unsigned int i = 0; i < teams.size(); i++)
        {
            gui->SetTeamScore(i, teams[i].score);
        }

        for (map<Point, Cell>::iterator it = mainField.begin(); it != mainField.end(); it++)
        {
            if (it->second.food > 0)
            {
                gui->SetFood(ConcreteFood(it->first, it->second.food));
            }
        }
    }
    gui->EndPaint();

    // scoreboard

/*    if (iRun % 100 == 0)
    {
        printf("SCORE after %d: ", iRun);
        for (unsigned int t = 0; t < teams.size(); t++)
        {
            printf("#%d(%ld); ", t, teams[t].score);
        }
        printf("\n");
    }*/
}

void AntManager::setGui(std::shared_ptr<antgui::IAntGui> gui)
{
	this->gui = gui;
}

void AntManager::setFoodGeneretor(std::shared_ptr<food_iterator> it)
{
	this->warehouse = it;
}
