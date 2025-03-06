#include "IconDatabase.h"
#include "../ProjectFilePathHandler.h"
#include "SummonerSpell.h"
#include "ItemDatabase.h"
#include "VersionDatabase.h"
#include "../Data/FileCopier.h"
#include "../Data/ETagDatabase.h"

const std::wstring IconDatabase::DEFAULT_FILE_PATH = ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Icons/");

#pragma warning(disable:4996)
void IconDatabase::DownloadIngameIcons(const char* version, ETagDatabase* iconHashes, std::function<void(const wchar_t*)> statusCallback) {
	std::shared_ptr<SynchronizedLogFile> logFile(new SynchronizedLogFile(ProjectFilePathHandler::Logs::GetDefaultFilePathUnicode() + std::wstring(L"icon_download.log")));
	ROSELogger logger;
	logger.setLogFileOutput(logFile);

	ResourceDownloaderSettings resourceSettings;
	statusCallback(L"[STATUS] Downloading minimap icons...");
	DownloadMinimapIcons(&resourceSettings, iconHashes, logger);
	statusCallback(L"[STATUS] Downloading spectator atlas icons...");
	DownloadSpectatorAtlasIcons(&resourceSettings, iconHashes, logger);
	statusCallback(L"[STATUS] Downloading trait icons...");
	DownloadTraitIcons(&resourceSettings, iconHashes, logger);
	statusCallback(L"[STATUS] Downloading spectator overview icons...");
	DownloadSpectatorOverviewIcons(&resourceSettings, iconHashes, logger);
	statusCallback(L"[STATUS] Downloading player ping icons...");
	DownloadPlayerPingIcons(&resourceSettings, iconHashes, logger);
	statusCallback(L"[STATUS] Downloading player position icons...");
	DownloadPlayerPositionIcons(&resourceSettings, iconHashes, logger);
	statusCallback(L"[STATUS] Downloading summoner spell icons...");
	DownloadSummonerSpellIcons(version, &resourceSettings, iconHashes, logger);
	statusCallback(L"[STATUS] Downloading SRX icons...");
	DownloadSRXIcons(&resourceSettings, iconHashes, logger);
}

void IconDatabase::DownloadIcons(const char* baseUrl, ETagDatabase* iconHashes, const std::vector<std::string>& iconsToDownload, ROSELogger& logger) {
	_wmkdir(DEFAULT_FILE_PATH.c_str());
	CURLBufferedWrapper wrapper("");
	for (auto iconToDownload : iconsToDownload) {
		char iconUrl[0x200] = { 0x00 };
		sprintf_s(iconUrl, baseUrl, iconToDownload.c_str());
		wrapper.setUrl(iconUrl);
		if (!wrapper.fireRequest()) {
			logger.logError("Error while downloading Icon '", iconToDownload.c_str(), "' from URL: ", iconUrl, " | Trying to use fallback copy...");
			std::string fallbackPath = ProjectFilePathHandler::Design::GetDefaultFilePath() + iconToDownload;
			std::string targetPath = (ProjectFilePathHandler::GetDefaultFilePath() + "Icons/") + iconToDownload;
			if (!FileCopier::Copy(std::wstring(fallbackPath.begin(), fallbackPath.end()).c_str(), std::wstring(targetPath.begin(), targetPath.end()).c_str())) {
				logger.logWarn("Fallback-Copy wasn't executable from ", fallbackPath.c_str(), "to ", targetPath.c_str());
			}
			else {
				logger.logInfo("Fallback-Copy successfully applied to target path: ", targetPath.c_str());
			}
			continue;
		}
		wchar_t savePath[0x200] = { 0x00 };
		swprintf_s(savePath, L"%s%hs", DEFAULT_FILE_PATH.c_str(), iconToDownload.c_str());
		FILE *fh = _wfopen(savePath, L"wb+");
		if (fh == nullptr) {
			int error = GetLastError();
			logger.logError("Error while creating icon ", savePath, ". Error: ", std::system_category().message(error).c_str());
			continue;
		}
		const char* data = wrapper.getReadData().c_str();
		fwrite(data, sizeof(char), wrapper.getReadDataLength(), fh);
		fclose(fh);
		logger.logInfo("Successfully saved URL '", iconUrl, "' as file: ", savePath);
	}
}

void IconDatabase::DownloadRankedIcons(ETagDatabase* iconHashes) {
	ROSELogger logger;
	ResourceDownloaderSettings resourceSettings;
	char url[0x300] = { 0x00 };
	DownloadIcons(resourceSettings.getValueUtf8(ResourceDownloaderSettings::RESOURCE_SHARED_COMPONENTS_BASE_URL, "").c_str(), iconHashes, {
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_UNRANKED, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_IRON, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_BRONZE, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_SILVER, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_GOLD, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_PLATIN, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_EMERALD, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_DIAMOND, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_MASTER, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_GRANDMASTER, "").c_str(),
		resourceSettings.getValueUtf8(ResourceDownloaderSettings::RANKED_ICON_CHALLENGER, "").c_str(),
	}, logger);

}

void IconDatabase::DownloadMinimapIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	std::vector<std::string> minimapIconNames = {
		settings->getValueUtf8(ResourceDownloaderSettings::CHEMTECH_DRAKE_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::CLOUD_DRAKE_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::ELDER_DRAKE_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::EPIC_MONSTER_ATLAS_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HEXTECH_DRAKE_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::INFERNAL_DRAKE_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::MOUNTAIN_DRAKE_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::OCEAN_DRAKE_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::BLUE_TURRET_ICON_NAME, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::RED_TURRET_ICON_NAME, "")
	};
	char iconBaseUrl[0x200] = { 0x00 };
	sprintf_s(iconBaseUrl, settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_BASE_URL, "").c_str(), settings->getValueUtf8(ResourceDownloaderSettings::ICON_URI, "").c_str());
	DownloadIcons(iconBaseUrl, iconHashes, minimapIconNames, logger);
}

void IconDatabase::DownloadTraitIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	std::vector<std::string> traitIconNames = {
		settings->getValueUtf8(ResourceDownloaderSettings::CS_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_KILLS_AND_DEATHS_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_MASTERY_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_VISION_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_WINRATE_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::KDA_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_DEATHS_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_FIRST_BLOOD_RATE_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_MASTERY_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_SURRENDER_RATE_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_VISION_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_WINRATE_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::OVERALL_AVERAGE_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::PENTAKILLER_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::SOLOKILLER_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::TEAMKILLER_ONLY_TRAIT_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_LANE_CS_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_LANE_CS_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_LANE_GOLD_DIFF_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::LOW_LANE_GOLD_DIFF_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_DAMAGE_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_SURRENDER_RATE_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::HIGH_FIRST_BLOOD_RATE_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::WILDCARD_TRAIT_ICON, ""),
	};
	char iconBaseUrl[0x200] = { 0x00 };
	sprintf_s(iconBaseUrl, settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_BASE_URL, "").c_str(), settings->getValueUtf8(ResourceDownloaderSettings::TRAITS_URI, "").c_str());
	DownloadIcons(iconBaseUrl, iconHashes, traitIconNames, logger);
}

void IconDatabase::DownloadPlayerPingIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	std::vector<std::string> playerPingIconNames = { 
		settings->getValueUtf8(ResourceDownloaderSettings::VS_ICON_NAME, "")	
	};
	char playerPingBaseUrl[0x200] = { 0x00 };
	sprintf_s(playerPingBaseUrl, settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_BASE_URL, "").c_str(), settings->getValueUtf8(ResourceDownloaderSettings::PINGS_URI, "").c_str());
	DownloadIcons(playerPingBaseUrl, iconHashes, playerPingIconNames, logger);
}

void IconDatabase::DownloadSpectatorOverviewIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	std::vector<std::string> spectatorOverviewIconNames = {
		settings->getValueUtf8(ResourceDownloaderSettings::SPECTATOR_GOLD_ICON, ""),
	};
	char spectatorOverviewIconBaseUrl[0x200] = { 0x00 };
	sprintf_s(spectatorOverviewIconBaseUrl, settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_BASE_URL, "").c_str(), settings->getValueUtf8(ResourceDownloaderSettings::SPECTATOR_URI, "").c_str());
	DownloadIcons(spectatorOverviewIconBaseUrl, iconHashes, spectatorOverviewIconNames, logger);

	std::vector<std::string> scoreboardIconNames = {
		settings->getValueUtf8(ResourceDownloaderSettings::ESPORTS_EPIC_AND_OBJECTIVES_KILLED_ICON, ""),
	};
	char scoreboardIconBaseUrl[0x200] = { 0x00 };
	sprintf_s(scoreboardIconBaseUrl, settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_BASE_URL, "").c_str(), settings->getValueUtf8(ResourceDownloaderSettings::SCOREBOARD_URI, "").c_str());
	DownloadIcons(scoreboardIconBaseUrl, iconHashes, scoreboardIconNames, logger);
}

void IconDatabase::DownloadSpectatorAtlasIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	std::vector<std::string> atlasIconNames = {
		settings->getValueUtf8(ResourceDownloaderSettings::SPECTATOR_GOLD_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::EPIC_MONSTER_ATLAS_NAME, ""),
	};
	char atlasIconsBaseUrl[0x200] = { 0x00 };
	sprintf_s(atlasIconsBaseUrl, settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_BASE_URL, "").c_str(), settings->getValueUtf8(ResourceDownloaderSettings::ATLAS_URI, "").c_str());
	DownloadIcons(atlasIconsBaseUrl, iconHashes, atlasIconNames, logger);
}

void IconDatabase::DownloadPlayerPositionIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	std::vector<std::string> positionIconNames = {
		settings->getValueUtf8(ResourceDownloaderSettings::PLAYER_POSITION_TOPLANER_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::PLAYER_POSITION_JUNGLER_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::PLAYER_POSITION_MIDLANER_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::PLAYER_POSITION_BOTLANER_ICON, ""),
		settings->getValueUtf8(ResourceDownloaderSettings::PLAYER_POSITION_SUPPORTER_ICON, ""),
	};
	DownloadIcons(settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_POSITION_BASE_URL, "").c_str(), iconHashes, positionIconNames, logger);
}


void IconDatabase::DownloadSRXIcons(ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	std::vector<std::string> srxIcons = {
		settings->getValueUtf8(ResourceDownloaderSettings::DRAGON_SOUL_SRX_ICON, ""),
	};
	char srxIconsBaseUrl[0x200] = { 0x00 };
	sprintf_s(srxIconsBaseUrl, settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_BASE_URL, "").c_str(), settings->getValueUtf8(ResourceDownloaderSettings::SRX_URI, "").c_str());
	DownloadIcons(srxIconsBaseUrl, iconHashes, srxIcons, logger);
}

void IconDatabase::DownloadSummonerSpellIcons(const char* version, ResourceDownloaderSettings* settings, ETagDatabase* iconHashes, ROSELogger& logger) {
	SummonerSpell::Init();
	std::vector<std::string> summonerSpellIconName;
	for (auto summonerSpellIt : SummonerSpell::GetSpellsSortedById()) {
		if (summonerSpellIt.first <= 0) {
			continue;
		}
		std::string summonerSpellIconPath = summonerSpellIt.second->getImagePath();
		std::string spellIconName = summonerSpellIconPath.substr(summonerSpellIconPath.find_last_of("\\/") + 1);
		summonerSpellIconName.push_back(spellIconName);
	}
	char summonerSpellUrl[0x200] = { 0x00 };
	std::string summonerSpellUrlString = settings->getValueUtf8(ResourceDownloaderSettings::RESOURCE_SUMMONERS_BASE_URL, "");
	sprintf_s(summonerSpellUrl, summonerSpellUrlString.c_str(), version, "%s");
	DownloadIcons(summonerSpellUrl, iconHashes, summonerSpellIconName, logger);
	SummonerSpell::OnDestroy();
}


void IconDatabase::DownloadItemIcons(ETagDatabase* iconHashes, std::function<void(uint32_t itemsRemaining, uint32_t totalAmountOfItems)> callbackOnItemFinishedDownloading) {
	DownloadItemIcons(VersionDatabase::getInstance()->getLatestVersion().c_str(), iconHashes, callbackOnItemFinishedDownloading);
}

void IconDatabase::DownloadItemIcons(const char* patchVersion, ETagDatabase* iconHashes, std::function<void(uint32_t itemsRemaining, uint32_t totalAmountOfItems)> callbackOnItemFinishedDownloading) {
	std::unordered_map<uint32_t, Item*> itemsToDownload = ItemDatabase::getInstance()->getItemList();
	std::mutex itemListMutex;
	std::thread threads[20];
	_wmkdir(DEFAULT_FILE_PATH.c_str());
	std::shared_ptr<SynchronizedLogFile> logFile(new SynchronizedLogFile(ProjectFilePathHandler::Logs::GetDefaultFilePathUnicode() + std::wstring(L"item_download.log")));
	ROSELogger logger;
	logger.setLogFileOutput(logFile);
	uint32_t totalAmountOfItems = (uint32_t)itemsToDownload.size();
	const char* itemUrl = "https://ddragon.leagueoflegends.com/cdn/%s/data/en_US/item.json";
	std::mutex etagMutex;
	std::map<std::wstring, std::string> etagMap;

	std::function<Item* (std::unordered_map<uint32_t, Item*>&)> extractItemFromList = [&itemListMutex](std::unordered_map<uint32_t, Item*>& itemsToDownload) {
		std::lock_guard<std::mutex> guard(itemListMutex);
		if (itemsToDownload.empty()) {
			return (Item*)nullptr;
		}
		Item* item = itemsToDownload.begin()->second;
		uint32_t itemId = item->getId();
		itemsToDownload.erase(itemId);
		return item;
	};

	for (uint32_t i = 0; i < 20; i++) {
		threads[i] = std::thread([&itemsToDownload, totalAmountOfItems, extractItemFromList, &logger, &itemListMutex, &etagMutex, &etagMap, &iconHashes, &callbackOnItemFinishedDownloading, patchVersion]() {
			CURLBufferedWrapper wrapper("");
			wrapper.setReuseConnectionEnabled(true);
			wrapper.setSslEnabled(true);
			Item* item = nullptr;
			while (!itemsToDownload.empty()) {
				uint32_t remainingDownloads = 0;
				uint32_t itemId = 0;
				std::string plainIconName;
				item = extractItemFromList(itemsToDownload);
				if (item == nullptr) {
					continue;
				}
				itemId = item->getId();
				plainIconName = item->getName();
				char currentIconName[0x50] = { 0x00 };
				char currentIconUrl[0x200] = { 0x00 };
				char iconUrl[0x200] = { 0x00 };
				char iconName[0x50] = { 0x00 };				
				sprintf_s(iconUrl, "https://ddragon.leagueoflegends.com/cdn/%s/img/item/%s", patchVersion, "%s");
				sprintf_s(iconName, "%s", "%i.png");
				sprintf_s(currentIconName, iconName, itemId);
				sprintf_s(currentIconUrl, iconUrl, currentIconName);
				wrapper.setHttpMethod(CURLWrapperHttpMethod::HEAD);
				wrapper.setUrl(currentIconUrl);
				if (!wrapper.fireRequest()) {
					std::lock_guard<std::mutex> guard(itemListMutex);
					logger.logError("Error while getting HEAD information from url: ", currentIconUrl);
					continue;
				}

				wchar_t savePath[0x100] = { 0x00 };
				swprintf_s(savePath, L"%s%hs", DEFAULT_FILE_PATH.c_str(), currentIconName);

				auto etag = wrapper.getETagFromLastRequest();
				if (_stricmp(iconHashes->getValueUtf8(savePath, "").c_str(), etag.c_str()) == 0) {
					logger.logInfo("Etag from icon ", savePath, " remained identical. Skipping..");
					{
						std::lock_guard<std::mutex> guard(itemListMutex);
						remainingDownloads = (uint32_t)itemsToDownload.size();
					}
					callbackOnItemFinishedDownloading(totalAmountOfItems - remainingDownloads, totalAmountOfItems);
					continue;
				}
				wrapper.setHttpMethod(CURLWrapperHttpMethod::GET);
				if (!wrapper.fireRequest()) {
					logger.logError("Error while downloading file: ", iconName, " from URL: ", iconUrl);
					continue;
				}
				FILE *fh = _wfopen(savePath, L"wb+");
				if (fh == nullptr) {
					logger.logError("Error while creating icon ", savePath, ". Error: ", std::system_category().message(GetLastError()).c_str());
					continue;
				}
				const char* data = wrapper.getReadData().c_str();
				fwrite(data, 1, wrapper.getReadDataLength(), fh);
				fclose(fh);
				logger.logInfo("Successfully item ", plainIconName.c_str(), " from URL ", wrapper.getUrl().c_str(), " as file: ", savePath);
				{
					std::lock_guard<std::mutex> guard(itemListMutex);
					remainingDownloads = (uint32_t)itemsToDownload.size();
					{
						std::unique_lock<std::mutex> lock(etagMutex);
						etagMap.insert_or_assign(std::wstring(savePath), etag);
					}
					callbackOnItemFinishedDownloading(totalAmountOfItems - remainingDownloads, totalAmountOfItems);
				}
			}
		});
	}
	for (uint32_t i = 0; i < 20; i++) {
		threads[i].join();
	}
	for (auto etagPair : etagMap) {
		auto& etag = etagPair.second;
		iconHashes->setValue(etagPair.first, std::wstring(etag.begin(), etag.end()).c_str());
	}
}