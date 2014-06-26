#include "tournament.hpp"
#include <future>
#include <iostream>
#include <memory>
#include <map>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <tuple>

using namespace std;

int Play::id_seq;
int Group::id_seq = 1;

void judge(Play* p, int server = 0)
{
	for (auto& i : p->players)
	{
		i.score = rand() % 100;
	}
//	return;
	stringstream title;
	vector<Player*> ps;
	ps.resize(p->players.size());
	int idx = 0;
	for (auto& i : p->players)
	{
		if (idx++) title << ",";
		title << "\"" << i.teamId << "\":\"" << i.participant->name << " (" << i.participant->id << ")\"";
		ps[i.teamId] = &i;
	}
	ofstream inputFile(to_string(p->id) + ".in");
	inputFile << "{" << title.str() << "}" << endl;
	for (auto& i : ps)
	{
		inputFile << i->participant->id << endl;
	}
	inputFile.close();

	setenv("ANTSTT_MAP", p->map.c_str(), 1);
	setenv("ANTSTT_STEPS", to_string(cfg()->steps()).c_str(), 1);

	FILE *results = popen(("./run-tt " + to_string(p->id) + " " + to_string(server)).c_str(), "r");
	if (!results)
	{
		cerr << "run-tt did not start: ";
		perror("popen()");
		for (auto& i : ps)
		{
			i->score = 0;
		}
		return;
	}
	map<string, int> k = {{"03", 7},{"04", 140},{"05", 1}, {"06", 4}, {"02", 1}};
	for (auto& i : ps)
	{
		if (fscanf(results, "%d", &(i->score)) != 1)
		{
			cerr << "run-tt did not write enough information for play " << p->id << "!" << endl << idx << endl;;
			i->score = 0;
			break;
		}
		i->score *= k[p->map];
	}
	fclose(results);
}

void generatePlays(Group* g, Participant* fakeParticipant)
{
	static vector<vector<int>> permutations = 
	{
		{ 0, 1, 2, 3 },
		{ 3, 0, 1, 2 },
		{ 2, 0, 3, 1 },
		{ 1, 2, 3, 0 },
		{ 0, 2, 1, 3 },
		{ 1, 3, 2, 0 }
	};
	vector<int> localPermutation { 0, 1, 2, 3 };
	random_shuffle(localPermutation.begin(), localPermutation.end());
	for (auto& j : cfg()->maps())
	{
		for (auto& i : permutations)
		{
			g->plays.emplace_back();
			Play& p = *(g->plays.rbegin());
			p.map = j;
			int idx = 0;
			for (auto& j : g->participants)
			{
				p.players.emplace_back(j.participant, localPermutation[i[idx++]]);
			}
			while (idx < 4)
			{
				p.players.emplace_back(fakeParticipant, localPermutation[i[idx++]]);
			}
			p.steps = cfg()->steps();
		}
	}
}

void Group::updateResults()
{
	map<Participant*, GroupParticipant*> ps;
	for (auto& i : participants)
	{
		ps.insert(make_pair(i.participant, &i));
		i.score = 0;
	}
	for (auto& i : plays)
	{
		for (auto& j : i.players)
		{
			auto it = ps.find(j.participant);
			if (it != ps.end())
			{
				it->second->score += j.score;
			}
		}
	}
	sort(participants.begin(), participants.end(),
		[](const GroupParticipant& a, const GroupParticipant& b)
		{
			return a.score > b.score;
		});
}

template<class T>
void shuffleVector(vector<T>& v)
{
	vector<T> t;
	int sz = v.size() & (~3);
	for (int i = 0; i < sz; ++i)
	{
		int idx = i / 4 + ((i % 4) & 1 ? sz / 4 : 0) + ((i % 4) & 2 ? sz / 2 : 0);
		t.emplace_back(move(v[idx]));
	}
	for (int i = sz; i < v.size(); ++i)
	{
		t.emplace_back(move(v[i]));
	}
	swap(v, t);
}

int main()
{
	Play::id_seq = cfg()->playIdSeq();
	srand(2014);
	vector<unique_ptr<Participant>> participants;
	for (auto& i : cfg()->getParticipants())
	{
		participants.emplace_back(new Participant(i.first, i.second));
	}

	cerr << "Loaded configuration for " << participants.size() << " participants" << endl;

	cerr << "Starting preliminary plays..." << endl;
	unique_ptr<Participant> fakeParticipant(new Participant("Robot", cfg()->preliminaryStrategy()));
	for (auto& i : participants)
	{
		cerr << "Processing player " << i->name << endl;
		Play *pp = i->preliminaryPlay = new Play;
		pp->map = cfg()->preliminaryMap();
		pp->players.emplace_back(&*fakeParticipant, 0);
		pp->players.emplace_back(&*i, 1);
		pp->players.emplace_back(&*fakeParticipant, 2);
		pp->players.emplace_back(&*i, 3);
		pp->steps = cfg()->steps();
		judge(pp);
		i->preliminaryScore = pp->players[1].score + pp->players[3].score;
	}
	cerr << "Preliminary plays have finished." << endl; 

	sort(participants.begin(), participants.end(),
		[](const unique_ptr<Participant>& a, const unique_ptr<Participant>& b)
		{
			return a->preliminaryScore > b->preliminaryScore;
		});
	
	shuffleVector(participants);
	for (auto& i : participants)
		cerr << i->name << " " << i->preliminaryScore << endl;

	vector<vector<unique_ptr<Group>>> groups;
	cerr << "Generating round 0 groups..." << endl;
	groups.emplace_back();
	unique_ptr<Group> curGroup(new Group);
	for (auto& i : participants)
	{
		if (curGroup->participants.size() == 4)
		{
			groups[0].emplace_back(move(curGroup));
			curGroup.reset(new Group);
		}
		curGroup->participants.emplace_back(&*i, nullptr);
	}
	if (curGroup->participants.size())
	{
		groups[0].emplace_back(move(curGroup));
	}
	cerr << "Round 0 groups have been generated." << endl;
	int server = 0;
	for (int round = 0; !round || groups[round - 1].size() > 1;)
	{
		cerr << "Generating round " << round << " plays..." << endl;
		for (auto& j : groups[round])
		{
			generatePlays(&*j, &*fakeParticipant);
		}
		cerr << "Round " << round << " plays have been generated." << endl;

		cerr << "Starting round " << round << " plays..." << endl;
		curGroup.reset(new Group);
		for (auto& j : groups[round])
		{
			cerr << "Starting group " << j->id << " plays..." << endl;
			vector<future<void>> results;
			for (int k = 0; k < j->plays.size(); ++k)
			{
				//auto& k : j->plays
				cerr << "Starting play " << j->plays[k].id << "..." << endl;
				results.emplace_back(async(std::launch::async, judge, &j->plays[k], k % 4));;
				if (k % 4 == 3)
				{
					for (auto& i : results)
					{
						i.get();
						cerr << "A play have finished." << endl;
					}
					results.clear();
				}
			}
			cerr << "Group " << j->id << " plays have finished." << endl;
			j->updateResults();
		}
		cerr << "Round " << round << " plays have been finished." << endl;
		
		int next = round + 1;
		cerr << "Generating round " << next << " groups." << endl;
		groups.emplace_back();
		vector<tuple<int, GroupParticipant*, Group*>> winners;
		for (auto& j : groups[round])
		{
			for (auto& i : j->participants)
			{
				winners.emplace_back(-i.score, &i, &*j);
				i.participant->score += i.score << round;
			}
		}
		sort(winners.begin(), winners.end());
		winners.resize((winners.size() + 1) / 2);
		for (auto& i : winners)
		{
			get<1>(i)->passed = true;
		}
		shuffleVector(winners);
		curGroup.reset(new Group);
		for (auto& i : winners)
		{
			if (curGroup->participants.size() == 4)
			{
				groups[next].emplace_back(move(curGroup));
				curGroup.reset(new Group);
			}
			curGroup->participants.emplace_back(get<1>(i)->participant, get<2>(i));
		}
		if (curGroup->participants.size())
		{
			groups[next].emplace_back(move(curGroup));
		}
		cerr << "Round " << next << " groups have been generated." << endl;
		round = next;
	}
	cerr << "All information was generated. Starting renderer..." << endl;
	groups.pop_back();
	vector<vector<Group*>> groups0;
	for (auto i = groups.rbegin(); i != groups.rend(); ++i)
	{
		vector<Group*> a;
		for (auto& j : *i)
		{
			a.emplace_back(&*j);
		}
		groups0.emplace_back(move(a));
	}
	unique_ptr<IRenderer> r(IRenderer::get());
	r->render(groups0);
	vector<Participant*> participants1;
	for (auto &i : participants)
	{
		participants1.emplace_back(&*i);
	}
	sort(participants1.begin(), participants1.end(),
		[](Participant* a, Participant* b)
		{
			return a->score > b->score;
		});
	r->renderParticipants(participants1);
}
