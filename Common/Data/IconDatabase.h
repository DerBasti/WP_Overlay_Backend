#ifndef __LEAGUE_ICON_DATABASE__
#define __LEAGUE_ICON_DATABASE__

#include "CURLWrapper.h"
#include "Team.h"
#include "../Logging/Logger.h"
#include <unordered_map>
#include "../IO/FunctionSettings.h"

class ETagDatabase;

class IconDatabase {
private:
	IconDatabase() {}
	static void DownloadIcons(const char* baseUrl, ETagDatabase* iconHashes, const std::vector<std::string>& iconsToDownload, ROSELogger& logger);
	static void DownloadMinimapIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static void DownloadPlayerPingIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static void DownloadTraitIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static void DownloadSpectatorAtlasIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static void DownloadSpectatorOverviewIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static void DownloadPlayerPositionIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static void DownloadSummonerSpellIcons(const char* version, ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static void DownloadSRXIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger);
	static const std::wstring DEFAULT_FILE_PATH;
public:
	virtual ~IconDatabase() {}

	static void DownloadIngameIcons(const char* version, ETagDatabase* iconHashes, std::function<void(const wchar_t*)> statusBarTextCallback);
	static void DownloadItemIcons(ETagDatabase* iconHashes, std::function<void(uint32_t itemsRemaining, uint32_t totalAmountOfItems)> callbackOnItemFinishedDownloading);
	static void DownloadItemIcons(const char* nonTrimmedPatchVersion, ETagDatabase* iconHashes, std::function<void(uint32_t itemsRemaining, uint32_t totalAmountOfItems)> callbackOnItemFinishedDownloading);
	static void DownloadRankedIcons(ETagDatabase* iconHashes);
};

#endif //__LEAGUE_ICON_DATABASE__