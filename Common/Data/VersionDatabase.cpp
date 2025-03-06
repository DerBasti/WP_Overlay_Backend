#include "VersionDatabase.h"
#include "json/document.h"
#include "CURLWrapper.h"

const char* versionsJsonUrl = "https://ddragon.leagueoflegends.com/api/versions.json";

using namespace rapidjson;
VersionDatabase* VersionDatabase::instance = nullptr;

VersionDatabase::VersionDatabase() {
	reloadVersions();
}

void VersionDatabase::reloadVersions() {
	versions.clear();
	CURLBufferedWrapper wrapper(versionsJsonUrl);
	wrapper.fireRequest();
	Document data;
	data.Parse(wrapper.getReadData().c_str());
	if (data.HasParseError()) {
		logger.logError("Error while parsing version data: ", wrapper.getLastResponseErrorMessage());
		return;
	}
	this->latestVersion = std::string(data.Begin()->GetString());
	for (auto it = data.Begin(); it != data.End(); it++) {
		this->versions.insert(std::move(std::string(it->GetString())));
	}
}