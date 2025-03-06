#ifndef __LEAGUE_VERSION_DATABASE__
#define __LEAGUE_VERSION_DATABASE__

#include <unordered_set>
#include <string>
#include "../Logging/Logger.h"

class VersionDatabase {
private:
	ROSEThreadedLogger logger;
	static VersionDatabase* instance;
	std::string latestVersion;
	std::unordered_set<std::string> versions;
	VersionDatabase();
public:
	virtual ~VersionDatabase() {}
	__inline static VersionDatabase* getInstance() {
		if (instance == nullptr) {
			instance = new VersionDatabase();
		}
		return instance;
	}

	static void ReleaseInstance() {
		if (instance) {
			delete instance;
			instance = nullptr;
		}
	}

	void reloadVersions();
	__inline std::string getLatestVersion() const {
		return latestVersion;
	}
	__inline std::wstring getLatestVersionTrimmedUnicode() const {
		std::string version = getLatestVersionTrimmed();
		std::wstring result(version.c_str(), version.c_str() + version.length());
		return result;
	}
	__inline std::string getLatestVersionTrimmed() const {
		return latestVersion.substr(0, latestVersion.length() - 2);
	}
	__inline bool isVersionExistent(const std::string& version) {
		return versions.find(version) != versions.cend();
	}
};

#endif //__LEAGUE_VERSION_DATABASE__