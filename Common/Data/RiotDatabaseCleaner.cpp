#include "RiotDatabaseCleaner.h"
#include "ChampionReader.h"
#include "ItemDatabase.h"
#include "SummonerSpell.h"
#include "VersionDatabase.h"

std::vector<std::function<void()>> RiotDatabaseCleaner::DeleteCallbacks;

void RiotDatabaseCleaner::AddDatabaseForCleanup(std::function<void()> deleteCallback) {
	DeleteCallbacks.push_back(std::move(deleteCallback));
}

void RiotDatabaseCleaner::CleanupDatabases() {
	for (auto& deleteFunc : DeleteCallbacks) {
		deleteFunc();
	}
}