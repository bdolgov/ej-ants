#ifndef TOURNAMENT_HPP
#define TOURNAMENT_HPP

#include <vector>
#include <string>

using std::string;
using std::vector;
using std::pair;

struct Play;

struct Participant
{
	string name; /* Имя для отображения */
	string id; /* id посылки в системе */

	Participant(const string& _name, const string& _id): name(_name), id(_id) {}

	int preliminaryScore = -1; /* Предварительный балл */
	Play* preliminaryPlay = 0; /* Указатель на предварительную игру */

	int score = 0;
};

struct Player
{
	Player(Participant* _participant, int _teamId): participant(_participant), teamId(_teamId) {}
	Participant *participant = nullptr;
	int score = -1; /* Набранные очки */
	int teamId = -1; /* Номер команды во время игры, \in [0, 3] */
};

struct Play
{
	vector<Player> players; /* Игроки */
	int steps = 0;
	static int id_seq;
	int id = id_seq++; /* id игры для json */
	string map;
};

struct Group;

struct GroupParticipant
{
	GroupParticipant(Participant* _participant, Group* _source): participant(_participant), source(_source) {}
	Participant *participant = nullptr; /* Участник */
	Group *source = nullptr; /* Группа, из которой он пришел, либо nullptr, если это неприменимо */
	int score = -1; /* Суммарный рейтинг */
	bool passed = false; /* Прошел ли в следующий раунд */
};

struct Group
{
	static int id_seq;
	int id = id_seq++; /* id группы (для ссылок) */
	vector<GroupParticipant> participants;  /* Участиники */
	
	vector<Play> plays;

	void updateResults();
};

struct Configuration
{
	virtual int playIdSeq() const = 0;
	virtual int steps() const = 0;
	virtual string preliminaryMap() const = 0;
	virtual string preliminaryStrategy() const = 0;
	virtual vector<string> maps() const = 0;
	virtual vector<pair<string, string>> getParticipants() const = 0;

	static Configuration* instance;
};
static Configuration* cfg() { return Configuration::instance; }

class IRenderer
{
	public:
		virtual void render(const vector<vector<Group*>>& groups) = 0;
		virtual void renderParticipants(const vector<Participant*>& participants) = 0;
		static IRenderer* get();
};

#endif
