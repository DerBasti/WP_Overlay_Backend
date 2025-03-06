#ifndef __LEAGUE_RIOT_INGAME_API__
#define __LEAGUE_RIOT_INGAME_API_

#include "RiotAPI.h"

class RiotIngameAPI : public RiotAPI {
private:

public:
	static const constexpr uint16_t DEFAULT_INGAME_PORT = 2999;
	static const constexpr wchar_t* DEFAULT_LEAGUE_INGAME_CLIENT_EXE_NAME = L"League Of Legends.exe";
	RiotIngameAPI();
	virtual ~RiotIngameAPI();
};

#endif