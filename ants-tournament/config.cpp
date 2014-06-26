#include "tournament.hpp"
#include <fstream>
struct Cfg : Configuration
{
	int playIdSeq() const { return 920000; }
	int steps() const { return 200; }
	string preliminaryMap() const { return "02"; }
	string preliminaryStrategy() const { return "407"; }
	vector<string> maps() const { return { "00", "02",/* "05", "06" */}; }

	vector<pair<string, string>> getParticipants() const;

} cfg0;

Configuration* Configuration::instance = &cfg0;

vector<pair<string, string>> Cfg::getParticipants() const
{
	vector<pair<string, string>> ret;
	std::ifstream f("participants.txt");
	char nameBuf[1024], idBuf[1024];
	while (f.getline(nameBuf, sizeof(nameBuf), ':') && f.getline(idBuf, sizeof(idBuf)))
	{
		ret.emplace_back(nameBuf, idBuf);
	}
	return ret;
}
