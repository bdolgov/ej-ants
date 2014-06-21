#ifndef IANTGUI_HPP
#define IANTGUI_HPP
#include <array>
#include <tuple>
#include <string>
#include <memory>
#include <sstream>
#include <vector>

namespace antgui
{
	using namespace std;

	enum GuiType {Qt, NCurses};

	class GUIException
	{
		string message;
	public:
		GUIException(const string& message) : message(message) {}

		const string& getMessage()
		{
			return message;
		}
	};

	struct Point
	{
		int x, y;
		bool operator< (const Point& p) const
		{
			if (x < p.x) return true;
			if (x > p.x) return false;
			if (y < p.y) return true;
			return false;
		}
		Point(int x, int y) : x(x), y(y) {}
		Point() {}
		string toString() const {
		    stringstream ss;
 		    ss << x << "," << y;
                    return ss.str();
		}
	};

	class Ant
	{
	public:
		virtual bool hasFood() const = 0;
		virtual bool isFrozen() const = 0;
		virtual Point getPoint() const = 0;
		virtual int getTeamId() const = 0;
		bool operator< (const Ant& other) const
		{
			if (isFrozen()) return false;
			if (other.isFrozen()) return true;
			if (hasFood()) return false;
			if (other.hasFood()) return true;
			return false;
		}
		typedef std::array<int, 5> arr_t;
		arr_t toArray() const {
			return { getPoint().x, getPoint().y, getTeamId(), hasFood(), isFrozen() };
		}
        virtual ~Ant() {}
	};

	class Food
	{
	public:
		virtual Point getPoint() const = 0;
		virtual int getCount() const = 0;
		virtual ~Food() {}
		typedef std::array<int, 3> arr_t;
		arr_t toArray() const {
			return { getPoint().x, getPoint().y, getCount() };
		}
 };

	class IAntGui
	{
	public:
		virtual void Clear() = 0;
        virtual void SetAnt(const Ant &ant) = 0;
		virtual void BeginPaint() = 0;
		virtual void EndPaint() = 0;
        virtual void SetFood(const Food &food) = 0;
		virtual void SetTeamScore(int teamId, int score) = 0;
		virtual void Paint() = 0;
        static std::shared_ptr<IAntGui> getGUI();
		virtual ~IAntGui() {}
	};
}
#endif
