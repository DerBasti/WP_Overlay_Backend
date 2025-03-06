#ifndef __RIOT_LOCK_FILE__
#define __RIOT_LOCK_FILE__

#include <string>
#include <memory.h>
#include <vector>
#include "../../Common/Logging/Logger.h"

class DataReader;

class RiotLockFile {
private:
	enum LockFileTokenId {
		CLIENT_NAME,
		PROCESS,
		PORT,
		SECRET,
		PROTOCOL,
		EXPECTED_AMOUNT,
		INVALID = -1
	};
	bool invalidFlag;
	std::vector<std::string> lockFileValues;
	ROSELogger logger;

	void readLockFile(DataReader& reader);
	void readValuesFromLockFile(std::shared_ptr<char>& lockFileContentShared);
public:

	RiotLockFile();
	RiotLockFile(const std::string& lolPath);
	RiotLockFile(const std::wstring& lolPath);
	virtual ~RiotLockFile();

	__inline bool isValid() const {
		return !invalidFlag;
	}
	__inline uint16_t getPort() const {
		return !isValid() ? 0 : static_cast<uint16_t>(atoi(lockFileValues.at(LockFileTokenId::PORT).c_str()));
	}
	__inline std::string getSecret() const {
		return !isValid() ? std::string("") : lockFileValues.at(LockFileTokenId::SECRET);
	}
};

#endif //__RIOT_LOCK_FILE__