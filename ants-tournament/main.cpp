#include "tournament.hpp"
#include <iostream>
#include <memory>
#include <map>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <fstream>

using namespace std;

int Play::id_seq;
int Group::id_seq = 1;
void judge(Play* p)
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
	ofstream inputFile("run.in");
	inputFile << "{" << title.str() << "}" << endl;
	for (auto& i : ps)
	{
		inputFile << i->participant->id << endl;
	}
	inputFile.close();

	setenv("ANTSTT_MAP", cfg()->tournamentMap().c_str(), 1);
	setenv("ANTSTT_ARGS", "-t", 1);

	FILE *results = popen(("./run-tt " + to_string(p->id)).c_str(), "r");
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
	for (auto& i : ps)
	{
		if (fscanf(results, "%d", &(i->score)) != 1)
		{
			cerr << "run-tt did not write enough information for play " << p->id << "!" << endl << idx << endl;;
			i->score = 0;
			break;
		}
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
	for (auto& i : permutations)
	{
		g->plays.emplace_back();
		Play& p = *(g->plays.rbegin());
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

int main()
{
	Play::id_seq = cfg()->playIdSeq();

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
		pp->players.emplace_back(&*fakeParticipant, 0);
		pp->players.emplace_back(&*i, 1);
		pp->players.emplace_back(&*fakeParticipant, 2);
		pp->players.emplace_back(&*i, 3);
		pp->steps = cfg()->steps();
		judge(pp);
		i->preliminaryScore = pp->players[1].score + pp->players[3].score;
	}
	cerr << "Preliminary plays have finished." << endl; 

	std::sort(participants.begin(), participants.end(),
		[](const unique_ptr<Participant>& a, const unique_ptr<Participant>& b)
		{
			return a->preliminaryScore > b->preliminaryScore;
		});

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

	for (int round = 0; groups[max(0, round - 1)].size() > 1;)
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
			for (auto& k : j->plays)
			{
				cerr << "Starting play " << k.id << "..." << endl;
				judge(&k);
				cerr << "Play " << k.id << " has finished." << endl;
			}
			cerr << "Group " << j->id << " plays have finished." << endl;
			j->updateResults();
		}
		cerr << "Round " << round << " plays have been finished." << endl;
		
		int next = round + 1;
		cerr << "Generating round " << next << " groups." << endl;
		groups.emplace_back();
		vector<pair<Participant*, Group*>> winners;
		for (auto& j : groups[round])
		{
			for (int i = 0; i < 2 && i < j->participants.size(); ++i)
			{
				winners.emplace_back(j->participants[i].participant, &*j);
			}
		}
		curGroup.reset(new Group);
		for (auto& i : winners)
		{
			if (curGroup->participants.size() == 4)
			{
				groups[next].emplace_back(move(curGroup));
				curGroup.reset(new Group);
			}
			curGroup->participants.emplace_back(i.first, i.second);
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
}
