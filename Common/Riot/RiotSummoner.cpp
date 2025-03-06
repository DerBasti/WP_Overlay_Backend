#include "RiotSummoner.h"
#include <Windows.h>
#include "../Logging/ErrorCodeTranslator.h"

RiotSummoner::RiotSummoner(std::string puuid, uint64_t accountId, uint64_t summonerId, std::string unicodeName) {
	this->puuid = puuid;
	this->accountId = accountId;
	this->summonerId = summonerId;
	this->accountNameUnicode = unicodeName;
	int len = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, accountNameUnicode.c_str(), (int)accountNameUnicode.length(), NULL, 0);
	encodedAccountName = std::wstring(len, 0);
	int result = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, accountNameUnicode.c_str(), (int)accountNameUnicode.length(), &encodedAccountName[0], len);
	if (!result) {
		DWORD lastError = GetLastError();
		logger.logWarn("UTF-Encoding for name '", accountNameUnicode.c_str(), "' failed. Error: ", ErrorCodeTranslator::GetErrorCodeString(lastError).c_str(), " [ID: 0x", ROSELogger::asHex(lastError), "].");
	}
}

RiotSummoner::~RiotSummoner() {

}
