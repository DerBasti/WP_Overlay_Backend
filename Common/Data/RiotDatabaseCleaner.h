#ifndef __LEAGUE_DATABASE_CLEANER__
#define __LEAGUE_DATABASE_CLEANER__

#include <functional>
#include <vector>

class RiotDatabaseCleaner {
private:
	RiotDatabaseCleaner() {}
	static std::vector<std::function<void()>> DeleteCallbacks;
public:
	virtual ~RiotDatabaseCleaner() {}

	static void AddDatabaseForCleanup(std::function<void()> deleteCallback);
	static void CleanupDatabases();
};

#endif //__LEAGUE_DATABASE_CLEANER__