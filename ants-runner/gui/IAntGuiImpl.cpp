#include "IAntGuiImpl.hpp"
#include <iostream>
#include <fstream>
namespace antgui
{

    void AntGuiLogImpl::SetAnt(const Ant &ant)
    {
	curAnts.push_back(ant.toArray());
/*        auto it = antMap.find(ant.getPoint());
        if (it == antMap.end() || *(it->second) < ConcreteAnt(ant))
        {
            antMap.insert(std::make_pair(ant.getPoint(), std::make_shared<ConcreteAnt>(ant)));
        }*/
    }
	
	string toJson(int x)
	{
		stringstream ss;
		ss << x;
		return ss.str();
	}
	
	template<int I, class... T>
	typename enable_if<I == sizeof...(T), string>::type tupleToJson (const tuple<T...>&);

	
	template<class T, size_t N>
	string toJson(const array<T, N>& v);

	template<class T>
	string toJson(const vector<T>& v);

	template<class... T>
	string toJson(const tuple<T...>& t);

	template<class T, size_t N>
	string toJson(const array<T, N>& v)
	{
		stringstream ss;
		ss << "[";
		for (size_t i = 0; i < N; ++i)
		{
			if (i) ss << ",";
			ss << toJson(v[i]);
		}
		ss << "]";
		return ss.str();
	}

	template<class T>
	string toJson(const vector<T>& v)
	{
		stringstream ss;
		ss << "[";
		for (auto i = v.begin(); i != v.end(); ++i)
		{
			if (i != v.begin()) ss << ",";
			ss << toJson(*i);
		}
		ss << "]";
		return ss.str();
	}

	template<int I, class... T>
	typename enable_if<I < sizeof...(T), string>::type tupleToJson(const tuple<T...>& t)
	{
		stringstream ss;
		if (I)
			ss << ",";
		ss << toJson(get<I>(t));
		return ss.str() + tupleToJson<I + 1>(t);
	}
	
	template<int I, class... T>
	typename enable_if<I == sizeof...(T), string>::type tupleToJson (const tuple<T...>&)
	{
		return "";
	}

	template<class... T>
	string toJson(const tuple<T...>& t)
	{
		return string("[") + tupleToJson<0>(t) + "]";
	}
	
    void AntGuiLogImpl::Paint()
    {
		vector<Ant::arr_t> ants;
		vector<Food::arr_t> foods;
		for (auto& i : antMap)
		{
			ants.emplace_back(i.second->toArray());
		}
		for (auto& i : foodMap)
		{
			foods.emplace_back(i.second->toArray());
		}
		ants = curAnts;
		std::sort(ants.begin(), ants.end(), [](const Ant::arr_t& a, const Ant::arr_t& b) { return a[2] < b[2]; });
		step_t step { score, move(ants), move(foods) };
		curAnts.clear();
		steps.emplace_back(move(step));
    }

	AntGuiLogImpl::~AntGuiLogImpl()
	{
		cout << toJson(steps);
	}
}
