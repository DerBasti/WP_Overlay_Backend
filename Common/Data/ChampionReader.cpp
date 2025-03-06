#include "ChampionReader.h"
#include "VersionDatabase.h"
#include "../ProjectFilePathHandler.h"
#include "../Data/ETagDatabase.h"
#include "../IO/FunctionSettings.h"
#include "CURLWrapper.h"
#include "../Logging/Logger.h"
#include "../Utility.h"
#include <mutex>

const std::wstring ChampionDatabase::DEFAULT_BASE_FOLDER = ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Champions/");
const std::wstring ChampionDatabase::DEFAULT_DESIGN_FOLDER = ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Design/");
const std::wstring ChampionDatabase::DEFAULT_SPLASH_FOLDER = DEFAULT_BASE_FOLDER + std::wstring(L"Splash/");
const std::wstring ChampionDatabase::DEFAULT_LOADING_SCREEN_FOLDER = DEFAULT_BASE_FOLDER + std::wstring(L"LoadingScreen/");
const std::wstring ChampionDatabase::DEFAULT_FACE_IMAGE_FOLDER = DEFAULT_BASE_FOLDER + std::wstring(L"Faces/");
ChampionImageLoadStrategy ChampionDatabase::IMAGE_LOAD_STRATEGY = ChampionImageLoadStrategy::LAZY_LOAD;

const char* championsJsonUrl = "http://ddragon.leagueoflegends.com/cdn/%s/data/en_US/champion.json";
ChampionDatabase* ChampionDatabase::instance = nullptr;
Champion* ChampionDatabase::CHAMPION_PLACEHOLDER = new Champion(0, "", "", (DEFAULT_DESIGN_FOLDER + std::wstring(L"no_champ.png")).c_str(), (DEFAULT_DESIGN_FOLDER + std::wstring(L"no_champ.png")).c_str(), (DEFAULT_DESIGN_FOLDER + std::wstring(L"no_champ.png")).c_str());

using namespace rapidjson;

ChampionDatabase::ChampionDatabase() {

	FunctionSettings funcSettings;
	std::wstring loadingType = funcSettings.getValue(FunctionSettings::CHAMPION_IMAGE_LOADING_TYPE, L"LAZY");
	ChampionDatabase::IMAGE_LOAD_STRATEGY = _wcsicmp(loadingType.c_str(), L"EAGER") == 0 ? ChampionImageLoadStrategy::EAGER_LOAD : ChampionImageLoadStrategy::LAZY_LOAD;

	Document championJson = getChampionData();
	Value& championData = championJson["data"];
	const static std::map<std::string, ChampionRoleType> STRING_TO_ROLE{
		{"Assassin", ChampionRoleType::ASSASSIN},
		{"Fighter", ChampionRoleType::FIGHTER},
		{"Mage", ChampionRoleType::MAGE},
		{"Marksman", ChampionRoleType::MARKSMAN},
		{"Support", ChampionRoleType::SUPPORT},
		{"Tank", ChampionRoleType::TANK},
	};
	for (auto it = championData.MemberBegin(); it != championData.MemberEnd(); it++) {
		Value& currentEntry = it->value;
		std::string name = currentEntry.FindMember("id")->value.GetString();
		uint32_t id = atoi(currentEntry.FindMember("key")->value.GetString());

		wchar_t buffer[0x200] = { 0x00 };
		swprintf_s(buffer, L"%s/%i.jpg", DEFAULT_SPLASH_FOLDER.c_str(), id);

		wchar_t loadingBuffer[0x200] = { 0x00 };
		swprintf_s(loadingBuffer, L"%s/%i.jpg", DEFAULT_LOADING_SCREEN_FOLDER.c_str(), id);

		wchar_t faceBuffer[0x200] = { 0x00 };
		swprintf_s(faceBuffer, L"%s/%i.png", DEFAULT_FACE_IMAGE_FOLDER.c_str(), id);

		auto ingameName = std::string(currentEntry.FindMember("name")->value.GetString());
		Champion* champ = new Champion(id, name.c_str(), ingameName, std::wstring(buffer), std::wstring(loadingBuffer), std::wstring(faceBuffer));
		auto categories = currentEntry["tags"].GetArray();
		std::vector<ChampionRoleType> roles;
		for (auto catIt = categories.Begin(); catIt != categories.End(); catIt++) {
			roles.push_back(STRING_TO_ROLE.at(catIt->GetString()));
		}
		champ->setChampionRoleTypes(roles);
		this->championById.insert(std::make_pair(id, champ));
		championByName.insert_or_assign(std::move(ingameName), champ);
		championByName.insert_or_assign(std::move(name), champ);
	}
	championById.insert(std::make_pair(0u, CHAMPION_PLACEHOLDER));
	championById.insert(std::make_pair(static_cast<uint32_t>(-1), CHAMPION_PLACEHOLDER));
	championByName.insert(std::make_pair("", CHAMPION_PLACEHOLDER));
#ifndef __LEAGUE_DIRECT2D_ENABLE__
	if (IMAGE_LOAD_STRATEGY == ChampionImageLoadStrategy::EAGER) {
		reloadImages();
	}
#endif
}

#ifndef _NO_DRAW
void ChampionDatabase::reloadAllChampionImages(Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer) {
	for (auto it : championById) {
		reloadChampionImageById(it.first, renderer);
	}
}

ChampionImages* ChampionDatabase::reloadChampionImageById(uint32_t id, Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer) {
	ChampionImages* result = nullptr;
	std::unique_lock<std::shared_mutex> lock(loadingMutex);
	if (championImages.find(id) == championImages.cend() && championById.find(id) != championById.cend()) {
		auto champion = championById.at(id);
		auto faceImage = Direct2DFactory::GetInstance()->createBitmap(champion->getPathToFaceImage().c_str(), renderer);
		auto splashImage = Direct2DFactory::GetInstance()->createBitmap(champion->getPathToSplash().c_str(), renderer);
		auto loadingImage = Direct2DFactory::GetInstance()->createBitmap(champion->getPathToLoadingScreen().c_str(), renderer);
		result = new ChampionImages(loadingImage, splashImage, faceImage);
		championImages.insert(std::make_pair(id, result));
	}
	return result;
}

void ChampionDatabase::reloadAllChampionImages(std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadCallback) {
	for (auto it : championById) {
		reloadChampionImageById(it.first, loadCallback);
	}
}
ChampionImages* ChampionDatabase::reloadChampionImageById(uint32_t id, std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadCallback) {
	ChampionImages* result = nullptr;
	std::unique_lock<std::shared_mutex> lock(loadingMutex);
	if (championImages.find(id) == championImages.cend() && championById.find(id) != championById.cend()) {
		auto champion = championById.at(id);
		auto faceImage = loadCallback(champion->getPathToFaceImage());
		auto splashImage = loadCallback(champion->getPathToSplash());
		auto loadingImage = loadCallback(champion->getPathToLoadingScreen());
		result = new ChampionImages(loadingImage, splashImage, faceImage);
		championImages.insert(std::make_pair(id, result));
	}
	return result;
}
#endif

void ChampionDatabase::unloadImages() {
#ifndef _NO_DRAW
	for (auto imageIt : championImages) {
		delete imageIt.second;
	}
	championImages.clear();
#endif
}

void ChampionDatabase::unloadChampions() {
	for (auto champIt : championById) {
		if (champIt.first == static_cast<uint32_t>(-1)) {
			continue;
		}
		delete champIt.second;
	}
	unloadImages();
	championById.clear();
	championByName.clear();
}

#ifndef __LEAGUE_DIRECT2D_ENABLE__
void ChampionDatabase::reloadImages() {
	unloadImages();
	reloadLoadingScreenImages();
	reloadSplashScreenImages();
}

void ChampionDatabase::reloadLoadingScreenImages() {
	for (auto it : championById) {
		uint32_t id = it.first;
		if (id == 0) {
			continue;
		}
		wchar_t loadingBuffer[0x200] = { 0x00 };
		swprintf_s(loadingBuffer, L"%s/%i.jpg", DEFAULT_LOADING_SCREEN_FOLDER.c_str(), id);
		championLoadingImages.insert(std::make_pair(id, new Gdiplus::Bitmap(loadingBuffer)));
	}
	championLoadingImages.insert(std::make_pair(CHAMPION_PLACEHOLDER->getId(), new Gdiplus::Bitmap(CHAMPION_PLACEHOLDER->getPathToLoadingScreen().c_str())));
}
void ChampionDatabase::reloadSplashScreenImages() {
	for (auto it : championById) {
		uint32_t id = it.first;
		if (id == 0) {
			continue;
		}
		wchar_t buffer[0x200] = { 0x00 };
		swprintf_s(buffer, L"%s/%i.jpg", DEFAULT_SPLASH_FOLDER.c_str(), id);
		championSplashImages.insert(std::make_pair(id, new Gdiplus::Bitmap(buffer)));
	}
	championSplashImages.insert(std::make_pair(CHAMPION_PLACEHOLDER->getId(), new Gdiplus::Bitmap(CHAMPION_PLACEHOLDER->getPathToSplash().c_str())));
}
#else
#ifndef _NO_DRAW
void ChampionDatabase::reloadImages(Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer) {
	unloadImages();
	if (IMAGE_LOAD_STRATEGY == ChampionImageLoadStrategy::EAGER_LOAD) {
		reloadAllChampionImages(renderer);
	}
}
void ChampionDatabase::reloadImages(std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadCallback) {
	unloadImages();
	if (IMAGE_LOAD_STRATEGY == ChampionImageLoadStrategy::EAGER_LOAD) {
		reloadAllChampionImages(loadCallback);
	}
}
#endif
#endif

ChampionDatabase* ChampionDatabase::getInstance() {
	if (instance == nullptr) {
		instance = new ChampionDatabase();
	}
	return instance;
}

std::string ChampionDatabase::getChampionUrl() {
	char buffer[0x500] = { 0x00 };
	sprintf_s(buffer, championsJsonUrl, VersionDatabase::getInstance()->getLatestVersion().c_str());
	return std::string(buffer);
}

Document ChampionDatabase::getChampionData() {
	auto championUrl = getChampionUrl();
	CURLBufferedWrapper wrapper(championUrl.c_str());
	bool success = wrapper.fireRequest();
	Document document;
	if (wrapper.getReadDataLength() > 0) {
		document.Parse(wrapper.getReadData().c_str());
	}
	return document;
}

#pragma warning(disable:4996)
void ChampionDatabase::downloadAndUpdateChampLoadingScreenImages(ETagDatabase* imageHashes, std::function<void(uint32_t, uint32_t)> callback) {
	std::unordered_map<uint32_t, Champion*> itemsToDownload = championById;
	std::mutex itemListMutex, etagListMutex;
	std::map<std::wstring, std::string> etagMap;
	std::thread threads[20];
	int mkDirResult = _wmkdir(DEFAULT_BASE_FOLDER.c_str());
	mkDirResult = _wmkdir(DEFAULT_LOADING_SCREEN_FOLDER.c_str());
	const uint32_t maxAmount = (uint32_t)championById.size();

	std::function<std::pair<uint32_t, std::string>(std::unordered_map<uint32_t, Champion*>&)> ExtractChampionNameToDownload = [&itemListMutex](std::unordered_map<uint32_t, Champion*>& champsToDownloadList) {
		std::lock_guard<std::mutex> guard(itemListMutex);
		if (champsToDownloadList.empty()) {
			return std::make_pair(0u, std::string());
		}
		uint32_t id = champsToDownloadList.begin()->first;
		std::string champName = champsToDownloadList.begin()->second->getName();
		champsToDownloadList.erase(champsToDownloadList.begin());
		return std::make_pair(id, champName);
	};

	std::function<std::string(const char*)> GenerateImageUrl = [](const char* champName) {
		char championUrl[0x200] = { 0x00 };
		sprintf_s(championUrl, "https://ddragon.leagueoflegends.com/cdn/img/champion/loading/%s_0.jpg", champName);
		return std::string(championUrl);
	};

	std::function<std::wstring(uint32_t)> GenerateSavePath = [](uint32_t champId) {
		wchar_t savePath[0x200] = { 0x00 };
		swprintf_s(savePath, L"%s/%i.jpg", DEFAULT_LOADING_SCREEN_FOLDER.c_str(), champId);
		return std::wstring(savePath);
	};

	std::function<bool(CURLWrapper& wrapper, ETagDatabase* imageHashes, std::wstring& savePath)> isEtagIdentical = [](CURLWrapper& wrapper, ETagDatabase* imageHashes, std::wstring& savePath) {
		wrapper.setHttpMethod(CURLWrapperHttpMethod::HEAD);
		bool success = wrapper.fireRequest();
		std::string etagFromRequest = wrapper.getETagFromLastRequest();
		std::string etagFromEtagDb = imageHashes->getValueUtf8(savePath, "");
		bool etagIdentical = (_stricmp(etagFromRequest.c_str(), etagFromEtagDb.c_str()) == 0) && !etagFromRequest.empty();
		return !success || etagIdentical;
	};

	std::function<bool(CURLWrapper& wrapper, const char*)> DownloadExecutor = [](CURLWrapper& wrapper, const char* champName) {
		wrapper.setHttpMethod(CURLWrapperHttpMethod::GET);
		return wrapper.fireRequest();
	};


	std::function<bool(CURLWrapper&, std::wstring& savePath, ROSELogger&)> FileSaver = [](CURLWrapper& downloader, std::wstring& savePath, ROSELogger& logger) {
		FILE *fh = _wfopen(savePath.c_str(), L"wb+");
		if (fh == nullptr) {
			logger.logError("Error while creating icon ", savePath, ". Error: ", std::system_category().message(GetLastError()).c_str());
			return false;
		}
		const char* data = downloader.getReadData().c_str();
		fwrite(data, sizeof(char), downloader.getReadDataLength(), fh);
		fclose(fh);
		return true;
	};
	std::shared_ptr<SynchronizedLogFile> logFile(new SynchronizedLogFile(ProjectFilePathHandler::Logs::GetDefaultFilePathUnicode() + std::wstring(L"champion_loadingscreen_downloads.log")));
	ROSEThreadedLogger logger;
	logger.setLogFileOutput(logFile);
	for (uint32_t i = 0; i < 20; i++) {
		threads[i] = std::thread([&]() {
			CURLBufferedWrapper downloader("");
			while (!itemsToDownload.empty()) {
				auto resultPair = ExtractChampionNameToDownload(itemsToDownload);
				uint32_t id = resultPair.first;
				std::string champName = resultPair.second;
				if (champName.length() <= 0) {
					continue;
				}
				uint32_t remainingItems = 0;
				{
					std::lock_guard<std::mutex> guard(itemListMutex);
					remainingItems = static_cast<uint32_t>(itemsToDownload.size());
				}
				callback(maxAmount - remainingItems, maxAmount);
				downloader.setUrl(GenerateImageUrl(champName.c_str()).c_str());
				std::wstring savePath = GenerateSavePath(id);
				if (isEtagIdentical(downloader, imageHashes, savePath)) {
					if (downloader.isLastRequestErrorPage()) {
						logger.logError("HEAD-request error while requesting loadingscreen image for '", champName.c_str(), "' from URL: ", downloader.getUrl().c_str());
					}
					else {
						logger.logInfo("Loadingscreen image ETag identical for Champion: ", champName.c_str());
					}
					continue;
				}
				std::string etag = downloader.getETagFromLastRequest();
				if (etag.empty()) {
					logger.logWarn("Etag was empty for Champion ", champName.c_str(), ".");
				}
				if (!DownloadExecutor(downloader, champName.c_str())) {
					logger.logError("Error while downloading icon for '", champName.c_str(), "' from URL: ", downloader.getUrl().c_str());
					continue;
				}
				if(FileSaver(downloader, savePath, logger)) {
					logger.logInfo("Successfully saved '", champName.c_str(), "' from URL ", downloader.getUrl().c_str(), " as file: ", savePath);
				}
				{
					std::unique_lock<std::mutex> lock(etagListMutex);
					etagMap.insert_or_assign(savePath, etag);
				}
			}
		});
	}
	for (uint32_t i = 0; i < 20; i++) {
		threads[i].join();
	}
	for (auto etagPair : etagMap) {
		auto& etag = etagPair.second;
		imageHashes->setValue(etagPair.first, std::wstring(etag.begin(), etag.end()).c_str());
	}
}


void ChampionDatabase::downloadAndUpdateChampSplashScreenImages(ETagDatabase* imageHashes, std::function<void(uint32_t, uint32_t)> callback) {

	std::unordered_map<uint32_t, Champion*> itemsToDownload = championById;
	std::mutex itemListMutex, etagListMutex;
	std::map<std::wstring, std::string> etagMap;
	std::thread threads[20];
	int mkDirResult = _wmkdir(DEFAULT_BASE_FOLDER.c_str());
	mkDirResult = _wmkdir(DEFAULT_SPLASH_FOLDER.c_str());
	std::string version = VersionDatabase::getInstance()->getLatestVersionTrimmed();
	const uint32_t maxAmount = (uint32_t)championById.size();
	
	std::function<std::pair<uint32_t, std::string>(std::unordered_map<uint32_t, Champion*>&)> ExtractChampionNameToDownload = [&itemListMutex](std::unordered_map<uint32_t, Champion*>& champsToDownloadList) {
		std::lock_guard<std::mutex> guard(itemListMutex);
		if (champsToDownloadList.empty()) {
			return std::make_pair(0u,std::string());
		}
		uint32_t id = champsToDownloadList.begin()->first;
		std::string champName = champsToDownloadList.begin()->second->getName();
		champsToDownloadList.erase(champsToDownloadList.begin());
		return std::make_pair(id, champName);
	};

	std::function<std::string(CURLWrapper&, std::pair<uint32_t, std::string>)> ExtractBaseSkinUrl = [](CURLWrapper& wrapper, std::pair<uint32_t, std::string> championData) {
		const char* jsonUrlTemplate = "https://raw.communitydragon.org/latest/plugins/rcp-be-lol-game-data/global/default/v1/champions/%i.json";
		char buffer[0x200] = { 0x00 };
		sprintf_s(buffer, jsonUrlTemplate, championData.first);
		wrapper.setUrl(buffer);
		wrapper.setHttpMethod(CURLWrapperHttpMethod::GET);
		std::string url;
		if (wrapper.fireRequest()) {
			rapidjson::Document championJson;
			championJson.Parse(wrapper.getReadData().c_str());
			auto skinsObj = championJson["skins"].GetArray()[0].GetObj();
			std::string splashPath = std::string(skinsObj["splashPath"].GetString());
			size_t pos = (std::min)(splashPath.find("Skins/"), splashPath.find("skins/"));
			splashPath = splashPath.substr(pos + 6);
			url = OverlayUtility::StringUtil::ToLower(splashPath);
			sprintf_s(buffer, "https://raw.communitydragon.org/latest/plugins/rcp-be-lol-game-data/global/default/assets/characters/%s/skins/%s", OverlayUtility::StringUtil::ToLower(championData.second).c_str(), url.c_str());
			url = std::string(buffer);
		}
		return url;
	};

	std::function<std::wstring(uint32_t)> GenerateSavePath = [](uint32_t champId) {
		wchar_t savePath[0x200] = { 0x00 };
		swprintf_s(savePath, L"%s/%i.jpg", DEFAULT_SPLASH_FOLDER.c_str(), champId);
		return std::wstring(savePath);
	};

	std::function<bool(CURLWrapper& wrapper, ETagDatabase* imageHashes, std::wstring& savePath)> isEtagIdentical = [](CURLWrapper& wrapper, ETagDatabase* imageHashes, std::wstring& savePath) {
		wrapper.setHttpMethod(CURLWrapperHttpMethod::HEAD);
		bool success = wrapper.fireRequest();
		bool etagIdentical = (_stricmp(wrapper.getETagFromLastRequest().c_str(), imageHashes->getValueUtf8(savePath, "").c_str()) == 0);
		return !success || etagIdentical;
	};

	std::function<bool(CURLWrapper& wrapper, std::string)> DownloadExecutor = [](CURLWrapper& wrapper, std::string url) {
		wrapper.setHttpMethod(CURLWrapperHttpMethod::GET);
		wrapper.setUrl(url.c_str());
		return wrapper.fireRequest();
	};

	std::function<bool(CURLWrapper&, std::wstring&, ROSELogger&)> FileSaver = [](CURLWrapper& downloader, std::wstring& savePath, ROSELogger& logger) {
		FILE *fh = _wfopen(savePath.c_str(), L"wb+");
		if (fh == nullptr) {
			logger.logError("Error while creating icon ", savePath, ". Error: ", std::system_category().message(GetLastError()).c_str());
			return false;
		}
		const char* data = downloader.getReadData().c_str();
		fwrite(data, sizeof(char), downloader.getReadDataLength(), fh);
		fclose(fh);
		logger.logInfo("Successfully saved URL ", downloader.getUrl().c_str(), " into file: ", savePath);
		return true;
	};

	std::shared_ptr<SynchronizedLogFile> logFile(new SynchronizedLogFile(ProjectFilePathHandler::Logs::GetDefaultFilePathUnicode() + std::wstring(L"champion_splashscreen_downloads.log")));
	ROSEThreadedLogger logger;
	logger.setLogFileOutput(logFile);
	for (uint32_t i = 0; i < 20; i++) {
		threads[i] = std::thread([&]() {
			CURLBufferedWrapper wrapper("");
			wrapper.setReuseConnectionEnabled(true);
			while (!itemsToDownload.empty()) {
				auto resultPair = ExtractChampionNameToDownload(itemsToDownload);
				uint32_t id = resultPair.first;
				std::string champName = resultPair.second;
				if (champName.length() <= 0) {
					continue;
				}
				uint32_t remainingItems = 0;
				{
					std::lock_guard<std::mutex> guard(itemListMutex);
					remainingItems = static_cast<uint32_t>(itemsToDownload.size());
				}
				callback(maxAmount - remainingItems, maxAmount);
				std::wstring savePath = GenerateSavePath(id);
				std::string url = ExtractBaseSkinUrl(wrapper, resultPair);
				wrapper.setUrl(url.c_str());
				if (isEtagIdentical(wrapper, imageHashes, savePath)) {
					std::lock_guard<std::mutex> guard(itemListMutex);
					if (wrapper.isLastRequestErrorPage()) {
						logger.logError("HEAD-request error while requesting splash screen image for '", champName.c_str(), "' from URL: ", wrapper.getUrl().c_str());
					}
					else {
						logger.logInfo("Splash screen image ETag identical for Champion: ", champName.c_str());
					}
					continue;
				}

				std::string etag = wrapper.getETagFromLastRequest();
				std::string previousEtag = imageHashes->getValueUtf8(savePath, "");
				if (!DownloadExecutor(wrapper, url)) {
					std::lock_guard<std::mutex> guard(itemListMutex);
					logger.logError("Error while downloading icon for '", champName.c_str(), "' from URL: ", wrapper.getUrl().c_str());
					continue;
				}
				if (FileSaver(wrapper, savePath, logger)) {
					logger.logInfo("Successfully saved '", champName.c_str(), "' from URL ", wrapper.getUrl().c_str(), " as file: ", savePath);
				}
				{
					std::unique_lock<std::mutex> lock(etagListMutex);
					etagMap.insert_or_assign(savePath, etag);
				}
			}
		});
	}
	for (uint32_t i = 0; i < 20; i++) {
		threads[i].join();
	}
	for (auto etagPair : etagMap) {
		auto& etag = etagPair.second;
		imageHashes->setValue(etagPair.first, std::wstring(etag.begin(), etag.end()).c_str());
	}
}

void ChampionDatabase::downloadAndUpdateChampFaceImages(ETagDatabase* imageHashes, std::function<void(uint32_t, uint32_t)> callback) {

	std::unordered_map<uint32_t, Champion*> itemsToDownload = championById;
	std::mutex itemListMutex, etagListMutex;
	std::map<std::wstring, std::string> etagMap;
	std::thread threads[20];
	int mkDirResult = _wmkdir(DEFAULT_BASE_FOLDER.c_str());
	mkDirResult = _wmkdir(DEFAULT_FACE_IMAGE_FOLDER.c_str());
	std::string version = VersionDatabase::getInstance()->getLatestVersionTrimmed();
	const uint32_t maxAmount = (uint32_t)championById.size();


	std::function<std::pair<uint32_t, std::string>(std::unordered_map<uint32_t, Champion*>&)> ExtractChampionNameToDownload = [&itemListMutex](std::unordered_map<uint32_t, Champion*>& champsToDownloadList) {
		std::lock_guard<std::mutex> guard(itemListMutex);
		if (champsToDownloadList.empty()) {
			return std::make_pair(0u, std::string());
		}
		uint32_t id = champsToDownloadList.begin()->first;
		std::string champName = champsToDownloadList.begin()->second->getName();
		champsToDownloadList.erase(champsToDownloadList.begin());
		return std::make_pair(id, champName);
	};

	std::function<bool(CURLWrapper& wrapper, uint32_t id)> DownloadExecutor = [&](CURLWrapper& wrapper, uint32_t id) {
		char championUrl[0x200] = { 0x00 };
		sprintf_s(championUrl, "https://raw.communitydragon.org/latest/plugins/rcp-be-lol-game-data/global/default/v1/champion-icons/%i.png", id);
		wrapper.setUrl(championUrl);
		return wrapper.fireRequest();
	};

	std::function<bool(CURLWrapper&, uint32_t, ROSELogger&)> FileSaver = [](CURLWrapper& downloader, uint32_t id, ROSELogger& logger) {
		wchar_t savePath[0x200] = { 0x00 };
		swprintf_s(savePath, L"%s/%i.png", DEFAULT_FACE_IMAGE_FOLDER.c_str(), id);
		FILE *fh = _wfopen(savePath, L"wb+");
		if (fh == nullptr) {
			logger.logError("Error while creating icon ", savePath, ". Error: ", std::system_category().message(GetLastError()).c_str());
			return false;
		}
		const char* data = downloader.getReadData().c_str();
		fwrite(data, sizeof(char), downloader.getReadDataLength(), fh);
		fclose(fh);
		logger.logInfo("Successfully saved URL ", downloader.getUrl().c_str(), " into file: ", savePath);
		return true;
	};

	std::shared_ptr<SynchronizedLogFile> logFile(new SynchronizedLogFile(ProjectFilePathHandler::Logs::GetDefaultFilePathUnicode() + std::wstring(L"champion_splashscreen_downloads.log")));
	ROSELogger logger;
	logger.setLogFileOutput(logFile);
	for (uint32_t i = 0; i < 20; i++) {
		threads[i] = std::thread([&itemsToDownload, ExtractChampionNameToDownload, DownloadExecutor, FileSaver, imageHashes, &logger, maxAmount, callback, &itemListMutex]() {
			CURLBufferedWrapper wrapper("");
			wrapper.setReuseConnectionEnabled(true);
			while (!itemsToDownload.empty()) {
				auto resultPair = ExtractChampionNameToDownload(itemsToDownload);
				uint32_t id = resultPair.first;
				std::string champName = resultPair.second;
				if (champName.length() <= 0) {
					continue;
				}
				std::string etag = wrapper.getETagFromLastRequest();
				if (!DownloadExecutor(wrapper, id)) {
					std::lock_guard<std::mutex> guard(itemListMutex);
					logger.logError("Error while downloading icon for '", champName.c_str(), "' from URL: ", wrapper.getUrl().c_str());
					uint32_t remainingItems = static_cast<uint32_t>(itemsToDownload.size());
					callback(maxAmount - remainingItems, maxAmount);
					continue;
				}
				FileSaver(wrapper, id, logger);
				{
					std::lock_guard<std::mutex> guard(itemListMutex);
					uint32_t remainingItems = static_cast<uint32_t>(itemsToDownload.size());
					callback(maxAmount - remainingItems, maxAmount);
				}
			}
		});
	}
	for (uint32_t i = 0; i < 20; i++) {
		threads[i].join();
	}
	for (auto etagPair : etagMap) {
		auto& etag = etagPair.second;
		imageHashes->setValue(etagPair.first, std::wstring(etag.begin(), etag.end()).c_str());
	}
}