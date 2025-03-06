#ifndef __LEAGUE_CHAMPION_READER__
#define __LEAGUE_CHAMPION_READER__

#include "json/document.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <shared_mutex>
#include <windows.h>
#include <map>
#include <objidl.h>
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef _NO_DRAW
#include <gdiplus.h>
using namespace Gdiplus;
#include "../GUI/Direct2DFactory.h"
#endif

#include <functional>
#include "../Riot/ChampMastery.h"

class ETagDatabase;

enum class ChampionRoleType : uint8_t {
	ASSASSIN,
	FIGHTER,
	MAGE,
	MARKSMAN,
	SUPPORT,
	TANK
};

class Champion {
private:
	uint32_t id;
	std::string name;
	std::string ingameName;
	std::wstring pathToSplash;
	std::wstring pathToLoadingScreen;
	std::wstring pathToFaceImage;
	std::vector<ChampionRoleType> categories;
public:
	Champion(uint32_t id, std::string name, std::string ingameName, std::wstring pathToSplash, std::wstring pathToLoadingScreen, std::wstring pathToFaceImage) {
		this->id = id;
		this->name = name;
		this->ingameName = ingameName;
		this->pathToSplash = pathToSplash;
		this->pathToLoadingScreen = pathToLoadingScreen;
		this->pathToFaceImage = pathToFaceImage;
	}
	virtual ~Champion() {

	}
	__inline uint32_t getId() const {
		return id;
	}
	__inline std::string getName() const {
		return name;
	}
	__inline std::string getIngameName() const {
		return ingameName;
	}
	__inline std::wstring getNameUnicode() const {
		return std::wstring(name.c_str(), name.c_str() + name.length());
	}
	__inline std::wstring getIngameNameUnicode() const {
		return std::wstring(ingameName.c_str(), ingameName.c_str() + ingameName.length());
	}
	__inline std::wstring getPathToSplash() const {
		return pathToSplash;
	}
	__inline std::wstring getPathToLoadingScreen() const {
		return pathToLoadingScreen;
	}
	inline std::wstring getPathToFaceImage() const {
		return pathToFaceImage;
	}
	inline const std::vector<ChampionRoleType>& getChampionRoleTypes() const {
		return categories;
	}
	inline void setChampionRoleTypes(const std::vector<ChampionRoleType>& roles) {
		categories = roles;
	}
};

enum class ChampionImageLoadStrategy : uint8_t {
	LAZY_LOAD,
	EAGER_LOAD
};

#ifndef _NO_DRAW

class ChampionImages {
private:
#ifndef __LEAGUE_DIRECT2D_ENABLE__
	Gdiplus::Image* loadingImage;
	Gdiplus::Image* splashImage;
	Gdiplus::Image* faceImage;
#else
	std::shared_ptr<Direct2DBitmap> loadingImage;
	std::shared_ptr<Direct2DBitmap> splashImage;
	std::shared_ptr<Direct2DBitmap> faceImage;
#endif
public:
	ChampionImages(std::shared_ptr<Direct2DBitmap> loading, std::shared_ptr<Direct2DBitmap> splash, std::shared_ptr<Direct2DBitmap> face) {
		loadingImage = loading;
		splashImage = splash;
		faceImage = face;
	}
	virtual ~ChampionImages() {

	}
	std::shared_ptr<Direct2DBitmap> getLoadingImage() const {
		return loadingImage;
	}
	std::shared_ptr<Direct2DBitmap> getSplashImage() const {
		return splashImage;
	}
	std::shared_ptr<Direct2DBitmap> getFaceImage() const {
		return faceImage;
	}
};

#endif


class ChampionDatabase {
private:
	std::shared_mutex loadingMutex;
	std::unordered_map<std::string, Champion*> championByName;
	std::unordered_map<uint32_t, Champion*> championById;
#ifndef _NO_DRAW
	std::unordered_map<uint32_t, ChampionImages*> championImages;
#endif

	std::unordered_map<uint32_t, std::unordered_map<std::string, PlayerChampionMastery>> championMasteries;
	std::unordered_map<std::string, std::map<uint64_t, PlayerChampionMastery>> championMasteriesPerSummoner;
	static ChampionDatabase* instance;
	std::string getChampionUrl();
	rapidjson::Document getChampionData();
	ChampionDatabase();
	static const std::wstring DEFAULT_BASE_FOLDER;
	static const std::wstring DEFAULT_DESIGN_FOLDER;
	static const std::wstring DEFAULT_SPLASH_FOLDER;
	static const std::wstring DEFAULT_FACE_IMAGE_FOLDER;
	static const std::wstring DEFAULT_LOADING_SCREEN_FOLDER;

#ifndef _NO_DRAW
	#ifndef __LEAGUE_DIRECT2D_ENABLE__
		void reloadLoadingScreenImages();
		void reloadSplashScreenImages();
		void reloadFaceImages();
	#else
		void reloadAllChampionImages(Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer);
		ChampionImages* reloadChampionImageById(uint32_t id, Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer);

		void reloadAllChampionImages(std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadCallback);
		ChampionImages* reloadChampionImageById(uint32_t id, std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadCallback);
	#endif
#endif
	void unloadChampions();
public:
	static ChampionImageLoadStrategy IMAGE_LOAD_STRATEGY;
	static Champion* CHAMPION_PLACEHOLDER;
	virtual ~ChampionDatabase() {
		unloadImages();
		unloadChampions();
	}
	static ChampionDatabase* getInstance();

	static void ReleaseInstance() {
		if (instance) {
			delete instance;
			instance = nullptr;
		}
	}


#ifndef _NO_DRAW
	#ifndef __LEAGUE_DIRECT2D_ENABLE__
		void reloadImages();
	#else
		void reloadImages(Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer);
		void reloadImages(std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadCallback);
	#endif
#endif
	void unloadImages();

	void downloadAndUpdateChampLoadingScreenImages(ETagDatabase* imageHashes, std::function<void(uint32_t, uint32_t)> callback);
	void downloadAndUpdateChampFaceImages(ETagDatabase* imageHashes, std::function<void(uint32_t, uint32_t)> callback);
	void downloadAndUpdateChampSplashScreenImages(ETagDatabase* imageHashes, std::function<void(uint32_t, uint32_t)> callback);

	std::wstring getChampionSplashById(uint32_t id) {
		return championById.find(id) == championById.cend() ? L"" : championById.at(id)->getPathToSplash();
	}

	void addMasteryForChampionFromSummoner(uint32_t champId, std::string summonerPuuid, uint64_t masteryAmount) {
		PlayerChampionMastery mastery(summonerPuuid, champId, masteryAmount);
		if (championMasteries.find(champId) == championMasteries.cend()) {
			championMasteries.insert({ champId, std::unordered_map<std::string, PlayerChampionMastery>{ {summonerPuuid, mastery}} });
		}
		else {
			std::unordered_map<std::string, PlayerChampionMastery> currentMastery = championMasteries.at(champId);
			currentMastery.insert({ summonerPuuid, mastery });
			championMasteries.insert_or_assign(champId, currentMastery);
		}

		std::map<uint64_t, PlayerChampionMastery> map;
		if (championMasteriesPerSummoner.find(summonerPuuid) != championMasteriesPerSummoner.cend()) {
			map = championMasteriesPerSummoner.at(summonerPuuid);
		}
		map.insert({ masteryAmount, mastery });
		championMasteriesPerSummoner.insert_or_assign(summonerPuuid, map);
	}
	inline uint64_t getMasteryPointOfChampForSummonerId(uint32_t champId, std::string summonerPuuid) {
		auto masteries = championMasteries.find(champId) != championMasteries.cend() ? championMasteries.at(champId) : std::unordered_map<std::string, PlayerChampionMastery>();
		return masteries.find(summonerPuuid) != masteries.cend() ? masteries.at(summonerPuuid).getMasteryAmount() : 0;
	}
	inline std::map<uint64_t, PlayerChampionMastery> getSortedMasteriesForAccount(std::string summonerPuuid) const {
		return (!summonerPuuid.empty() && championMasteriesPerSummoner.find(summonerPuuid) != championMasteriesPerSummoner.cend() ? championMasteriesPerSummoner.at(summonerPuuid) : std::map<uint64_t, PlayerChampionMastery>());
	}

	__inline bool isChampionName(const char* name) const {
		return championByName.find(std::string(name)) != championByName.cend();
	}
	__inline Champion* getChampionById(uint32_t id) const {
		return championById.find(id) != championById.cend() ? championById.at(id) : nullptr;
	}
	__inline Champion* getChampionByName(const char* name) const {
		return isChampionName(name) ? championByName.at(std::string(name)) : nullptr;
	}
	inline const std::unordered_map<uint32_t, Champion*>& getChampionMap() const {
		return championById;
	}

#ifndef _NO_DRAW
	#ifndef __LEAGUE_DIRECT2D_ENABLE__
		__inline Gdiplus::Image* getSplashImageForChamp(uint32_t id) {
			return championSplashImages.find(id) != championSplashImages.cend() ? championSplashImages.at(id) : nullptr;
		}
		__inline Gdiplus::Image* getLoadingImageForChamp(uint32_t id) {
			return championLoadingImages.find(id) != championLoadingImages.cend() ? championLoadingImages.at(id) : nullptr;
		}
		__inline Gdiplus::Image* getFaceImageForChamp(uint32_t id) {
			return championFaceImages.find(id) != championFaceImages.cend() ? championFaceImages.at(id) : nullptr;
		}
	#else
		ChampionImages* getImagesForChamp(uint32_t id, Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer) {
			ChampionImages *result = nullptr;
			{
				std::shared_lock<std::shared_mutex> lock(loadingMutex);
				result = championImages.find(id) != championImages.cend() ? championImages.at(id) : nullptr;
			}
			if (!result && IMAGE_LOAD_STRATEGY == ChampionImageLoadStrategy::LAZY_LOAD) {
				result = this->reloadChampionImageById(id, renderer);
			}
			return result;
		}
		__inline std::shared_ptr<Direct2DBitmap> getSplashImageForChamp(uint32_t id) {
			std::shared_lock<std::shared_mutex> lock(loadingMutex);
			return championImages.find(id) != championImages.cend() ? championImages.at(id)->getSplashImage() : nullptr;
		}
		__inline std::shared_ptr<Direct2DBitmap> getLoadingImageForChamp(uint32_t id) {
			std::shared_lock<std::shared_mutex> lock(loadingMutex);
			return championImages.find(id) != championImages.cend() ? championImages.at(id)->getLoadingImage() : nullptr;
		}
		__inline std::shared_ptr<Direct2DBitmap> getFaceImageForChamp(uint32_t id) {
			std::shared_lock<std::shared_mutex> lock(loadingMutex);
			return championImages.find(id) != championImages.cend() ? championImages.at(id)->getFaceImage() : nullptr;
		}
	#endif
	__inline bool isImagesLoaded() const {
		return IMAGE_LOAD_STRATEGY == ChampionImageLoadStrategy::EAGER_LOAD && !championImages.empty();
	}
	inline static ChampionImageLoadStrategy GetLoadingStrategyFromString(const std::wstring& stratString) {
		return _wcsicmp(stratString.c_str(), L"EAGER") == 0 ? ChampionImageLoadStrategy::EAGER_LOAD : ChampionImageLoadStrategy::LAZY_LOAD;
	}
	inline static std::wstring GetLoadingStrategyString(ChampionImageLoadStrategy strat) {
		return (strat == ChampionImageLoadStrategy::EAGER_LOAD ? L"EAGER" : L"LAZY");
	}
#endif
};

#endif //__LEAGUE_CHAMPION_READER__