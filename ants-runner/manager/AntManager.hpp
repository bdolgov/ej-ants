#ifndef ANTMANAGER_HPP
#define ANTMANAGER_HPP

//#include "Ant.hpp"

#include "IAntGUI.hpp"
#include "IAntLogic.hpp"

#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <fstream>


#include <stdio.h>
#include <stdlib.h>

typedef antgui::Point Point;

using std::vector;
using std::map;

const int dx[] = {-1, 1,  0, 0, 0};
const int dy[] = {0,  0, -1, 1, 0};
const int df[] = {-5,  -5, -5, -5, 0};

struct GameSettings {
    std::string mapName;
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

class food_iterator1 : public std::iterator< std::input_iterator_tag, ConcreteFood>
{
	std::ifstream map;
	bool eof;
	public:
		food_iterator1(): eof(true) {}
		food_iterator1(const std::string& s): map(s), eof(false) {}
		
		bool operator==(const food_iterator1& other) {
			return map.eof() == other.eof;
		}
		
		ConcreteFood operator*()
		{
			int x, y, i;
			map >> x >> y >> i;
			return {{x, y}, i};
		}
		
		food_iterator1& operator++()
		{
			return *this;
		}

		food_iterator1& operator++(int)
		{
			return *this;
		}
};

typedef food_iterator0 food_iterator;


struct MetaAnt : public antgui::Ant, public antlogic::Ant
{
    int           teamId;
    antgui::Point pos;
    char          mem[antlogic::MAX_MEMORY];
    bool          frozen;
    int           ftime;
    bool          withFood;

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
};

struct Cell
{
    int food;

    vector<MetaAnt*> ants;

    int hillId;

    int smell;
    int smellIntensity;

    bool isHill;
    bool isFood;
    bool isWall;
    bool isAnt;

    Cell() : food(0), hillId(-1),
             smell(0), smellIntensity(0),
             isHill(false), isFood(false),
             isWall(false), isAnt(false) {}

    antlogic::AntSensor sense(int teamId) const;
};

struct Team
{
    long score;
    vector<MetaAnt> ants;

    Team() : score(0) {}
};

typedef map<Point, Cell> Field;

class AntManager
{
    int maxAntCountPerTeam;
    int teamCount;
    int height;
    int width;
    Field mainField;
    GameSettings settings;
    std::shared_ptr<antgui::IAntGui> gui;
    vector<std::shared_ptr<antlogic::IAntLogic> > ai;

    std::shared_ptr<food_iterator> warehouse;

    vector<Team> teams;

private:
    Point hillOf(int teamId);

public:
    AntManager(const GameSettings& settings,
               int height,
               int width,
               int maxAntCountPerTeam);

    void step(int iRun);
    void setGui(std::shared_ptr<antgui::IAntGui> gui);
    void setFoodGeneretor(std::shared_ptr<food_iterator> it);
};

#endif
