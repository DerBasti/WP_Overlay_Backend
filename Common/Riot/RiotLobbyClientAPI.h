#ifndef __LEAGUE_RIOT_LOBBY_CLIENT_API__
#define __LEAGUE_RIOT_LOBBY_CLIENT_API__

#include "RiotAPI.h"

class RiotLobbyClientAPI : public RiotAPI {
private:
	RiotLockFile riotLockFile;
	std::wstring leagueDirectory;
	void parseAndAddRequestArgs();
public:
	static const constexpr wchar_t* DEFAULT_LEAGUE_LOBBY_CLIENT_EXE_NAME = L"LeagueClientUX.exe";

	RiotLobbyClientAPI(const wchar_t* leagueDirectory);
	RiotLobbyClientAPI(const RiotLobbyClientAPI& api) = delete;

	virtual ~RiotLobbyClientAPI() {

	}

	inline std::wstring getLeagueDirectory() const {
		return leagueDirectory;
	}
	__inline bool isAvailable() const {
		return riotLockFile.isValid();
	}

};

#endif