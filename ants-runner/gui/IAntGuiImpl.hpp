#ifndef ANTGUIIMPL_HPP
#define ANTGUIIMPL_HPP

#include "IAntGUI.hpp"

#include <map>
#include <stack>
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>

#define ANT_SIZE 15

using namespace std;

namespace antgui
{
    class AntGuiLogImpl;

    class ConcreteAnt : public Ant
    {
    private:
        bool food;
        bool frozen;
        Point point;
        int teamId;
    public:
        ConcreteAnt(bool food, bool frozen, Point &p, int teamId)
            : food(food), frozen(frozen), point(p), teamId(teamId) {}
        ConcreteAnt(const Ant &ant) {
            food = ant.hasFood();
            frozen = ant.isFrozen();
            point = ant.getPoint();
            teamId = ant.getTeamId();
        }

        bool hasFood() const { return food;}
        bool isFrozen() const { return  frozen;}
        Point getPoint() const {return point;}
        int getTeamId() const { return teamId;}

        virtual ~ConcreteAnt() {

        }
    };


    class ConcreteFood : public antgui::Food
    {
        Point p;
        int count;
    public:
        ConcreteFood() {}
        ConcreteFood(const Point &p, int count) : p(p), count(count) {}
        ConcreteFood(const Food &food) {
            p = food.getPoint();
            count = food.getCount();
        }

        virtual Point getPoint() const
        {
            return p;
        }

        virtual int getCount() const
        {
            return count;
        }

        virtual ~ConcreteFood()
        {

        }
    };




	class AntGuiLogImpl : public IAntGui
	{
        map<Point, std::shared_ptr<const Food> > foodMap;
        map<Point, std::shared_ptr<const Ant> > antMap;
	vector<Ant::arr_t> curAnts;
		vector<int> score;
		typedef tuple<vector<int>, vector<Ant::arr_t>, vector<Food::arr_t>> step_t;
		vector<step_t> steps;

	public:
//        map<int, QColor> teamColor;

        void SetTeamScore(int teamId, int score)
        {
			this->score[teamId] = score;
            //widget->getLabel(teamId).setText(QString("team: %1 score: %2").arg(teamId).arg(score));
        }

		AntGuiLogImpl(const std::string& path)
		{
			score.resize(4);
		}
		
		~AntGuiLogImpl();

		virtual void Clear()
		{
            antMap.clear();
            foodMap.clear();
		}

        void init()
		{
		//	teamColor.insert(pair<int, QColor>(0, QColor(Qt::red)));
		//	teamColor.insert(pair<int, QColor>(1, QColor(Qt::green)));
		//	teamColor.insert(pair<int, QColor>(2, QColor(Qt::blue)));
		//	teamColor.insert(pair<int, QColor>(3, QColor(Qt::yellow)));
		}

        virtual void SetAnt(const Ant &ant);

        virtual void SetFood(const Food &food)
		{
			foodMap.erase(food.getPoint());
			foodMap.insert(std::make_pair(food.getPoint(), std::make_shared<ConcreteFood>(food)));
		}

		virtual void Paint();

		virtual void BeginPaint()
		{
//			paintLock.lock();
			Clear();
		}

		virtual void EndPaint()
		{
//			paintLock.unlock();
		}
	};



}

#endif
