#include "RiotLobbyClientAPI.h"
#include "Base64Authentication.h"

RiotLobbyClientAPI::RiotLobbyClientAPI(const wchar_t* leagueDirectory) : RiotAPI(RiotLockFile(leagueDirectory).getPort()) {
	riotLockFile = RiotLockFile(leagueDirectory);
	this->leagueDirectory = std::wstring(leagueDirectory);
	parseAndAddRequestArgs();
}

void RiotLobbyClientAPI::parseAndAddRequestArgs() {
	std::string auth = std::string("riot:") + riotLockFile.getSecret();
	Base64 authAsBase64(auth.c_str());
	std::vector<std::string> arguments;
	char authBuffer[0x200] = { 0x00 };
	sprintf_s(authBuffer, "Authorization: Basic %s", authAsBase64.getBase64String());
	arguments.push_back(authBuffer);
	char jsonBuffer[0x200] = { 0x00 };
	arguments.push_back("Accept: application/json");
	this->addArgumentsToDataRequests(arguments);
}