#ifndef ANTMANAGER_HPP
#define ANTMANAGER_HPP

//#include "Ant.hpp"

#include "IAntGUI.hpp"
#include "IAntLogic.hpp"

#include <stack>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <fstream>

using namespace std;

#include <stdio.h>
#include <stdlib.h>

typedef antgui::Point Point;

using std::vector;
using std::map;

const int dx[] = {-1, 1,  0, 0, 0};
const int dy[] = {0,  0, -1, 1, 0};
const int df[] = {-5,  -5, -5, -5, 0};

struct GameSettings {
	std::string mapName, dumpScore;
	std::vector<std::string> players;
	int stepLimit, timeLimit;
};

class ConcreteFood : public antgui::Food
{
	Point p;
	int count;
public:
	ConcreteFood() {}
	ConcreteFood(const Point &p, int count) : p(p), count(count) {}
	ConcreteFood(const antgui::Food *food) {
		if (food) {
		   p = food->getPoint();
		   count = food->getCount();
		}
	}

	virtual Point getPoint() const
	{
		return p;
	}

	virtual int getCount() const
	{
		return count;
	}
};

class food_iterator0: public std::iterator< std::input_iterator_tag, ConcreteFood>
{
	enum {fractal_den = 3};
	struct FuncParam
	{
		int n;
		int fsize;
		Point p;
		int count;
		FuncParam(int n, const Point &p, int count, int fsize)
			: n(n), fsize(fsize), p(p), count(count) {}
	};
	std::stack<FuncParam> s;
	int size;
	int iteration;
public:

	food_iterator0() : iteration(-1) {}
	food_iterator0(const food_iterator0& it);
	food_iterator0(int size);

	ConcreteFood operator*();
	food_iterator0& operator++();
	food_iterator0 operator++(int);

private:
	void WakeUp();
	ConcreteFood GetFood()
	{
		return ConcreteFood(s.top().p, s.top().count);
	}

	friend bool operator!=(const food_iterator0& it1, const food_iterator0& it2);
};

typedef food_iterator0 food_iterator;

struct MetaAnt : public antgui::Ant, public antlogic::Ant
{
	bool dead = false;
	int		   teamId;
	antgui::Point pos;
	unique_ptr<char[]> mem;
	int		   ftime = 0;
	bool		  withFood = false;

	MetaAnt(int teamId = 0, Point startPos = Point(1, 1));

	// gui needs following
	bool isFrozen() const;
	Point getPoint() const;

	// logic needs following
	char* getMemory() const;

	// both need following
	bool hasFood() const;
	int getTeamId() const;

	friend class AntManager;

	void tick();
};

struct Team;

struct Cell
{
	int food = 0;

	set<MetaAnt*> ants;

	int hillId = -1;

	int smell = 0;
	int smellIntensity = 0;

	bool isWall = false;

	antlogic::AntSensor sense(Team* team) const;

	void tick();
};

struct Team
{
	vector<unique_ptr<MetaAnt>> ants;
	shared_ptr<antlogic::IAntLogic> ai;
	Point hill;
	int id;

	Team(int _id) : id(_id) {}
};

typedef map<Point, Cell> Field;

class AntManager
{
	vector<unique_ptr<MetaAnt>> ants;

	int maxAntCountPerTeam;
	int height;
	int width;
	int curStep;
	Field mainField;
	GameSettings settings;
	std::shared_ptr<antgui::IAntGui> gui;
	vector<unique_ptr<Team>> teams;

	std::shared_ptr<food_iterator> warehouse;

private:
	Point hillOf(int teamId);
	void move(MetaAnt* ant, int dx, int dy);
	void bite(MetaAnt* ant, int dx, int dy);
	void food(MetaAnt* ant, bool);
public:
	AntManager(const GameSettings& settings,
			   int height,
			   int width,
			   int maxAntCountPerTeam);

	void step();
	void setGui(std::shared_ptr<antgui::IAntGui> gui);
	int score(int team);
};

#endif
