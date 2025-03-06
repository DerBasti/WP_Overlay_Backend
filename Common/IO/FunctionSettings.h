#ifndef __LEAGUE_FUNCTION_SETTINGS__
#define __LEAGUE_FUNCTION_SETTINGS__

#include "FileReader.h"
#include "../Logging/Logger.h"
#include <map>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <set>

class Settings;

class SettingsKeyChangedListener {
private:
	std::unordered_map<std::wstring, std::function<void(const std::wstring&)>> keysToListenFor;
	std::set<Settings*> settingsListenedTo;
protected:
	virtual void addKeyToListenFor(Settings* settings, const std::wstring& key, std::function<void(const std::wstring&)> callback);
public:
	SettingsKeyChangedListener();
	virtual ~SettingsKeyChangedListener();

	virtual void onSettingsKeyValueChanged(const std::wstring& key, const std::wstring& newValue);
};

class Settings {
private:
	ROSEThreadedLogger logger;
	mutable std::shared_mutex settingsMapMutex;
	std::wstring settingsFilePath;
	std::string lastError;
	std::map<std::wstring, std::wstring> commentsForSettings;
	std::map<std::wstring, std::wstring> settingsMap;
	std::multimap<SettingsKeyChangedListener*, std::wstring> listenerMap;
	std::multimap<std::wstring, SettingsKeyChangedListener*> listenedKeysMap;
	void readSettingsFromFile(FILE* fh);
protected:
	bool isSuccessfullyLoaded() {
		return !settingsMap.empty();
	}
	virtual bool setDefaultSettings() {
		return false;
	}
	virtual void addCommentForKey(std::wstring key, std::wstring comment);
	bool setValueIfAbsent(std::wstring key, const wchar_t* value, std::wstring comment = L"");
	void notifyListenerForKey(const std::wstring& key, const std::wstring& value) const;
public:
	Settings(const wchar_t* filePath);
	virtual ~Settings();

	void updateSettingsFile();
	void reloadSettingsFile();

	void addListenerForKey(std::wstring keyToListenFor, SettingsKeyChangedListener* listener);
	void removeAllListenerForCaller(SettingsKeyChangedListener* callerPtr);

	template<class _T>
	void setValueFromInt(std::wstring key, _T value) {
		wchar_t buffer[0x60] = { 0x00 };
		if (value < 0) {
			::_i64tow_s((long long)value, buffer, 0x60, 10);
		}
		else {
			::_ui64tow_s((unsigned long long)value, buffer, 0x60, 10);
		}
		setValue(key, buffer);
	}
	void setValue(std::wstring key, const wchar_t* value);

	template<class _T>
	_T getValueAsInt(std::wstring key, _T defaultValue) const {
		auto valueAsString = getValue(key, L"");
		uint64_t value = (!valueAsString.empty() && valueAsString.front() == '-') ? _wcstoi64(valueAsString.c_str(), nullptr, 10) : _wcstoui64(valueAsString.c_str(), nullptr, 10);
		return valueAsString.length() > 0 ? (_T)value : defaultValue;
	}

	virtual bool isValueAbsent(std::wstring key) const {
		return settingsMap.find(key) == settingsMap.cend();
	}

	std::wstring getValue(std::wstring key, const wchar_t* defaultValue) const;
	std::string getValueUtf8(std::wstring key, const char* defaultValue) const;
};

class FunctionSettings : public Settings {
protected:
	virtual bool setDefaultSettings();
public:
	static constexpr const wchar_t* BANNER_GAME_EVENT_NAME = L"feature.banner.event.name";
	static constexpr const wchar_t* BANNER_GAME_EVENT_DESCRIPTION = L"feature.banner.event.description";
	static constexpr const wchar_t* GAME_AMOUNT_TO_PLAY_IN_SESSION = L"feature.overview.bestof.amount";
	static constexpr const wchar_t* LATEST_PATCH_VERSION = L"feature.patch.lastversion";

	static constexpr const wchar_t* PRIMELEAGUE_URL_STARTER_TOKEN = L"feature.primeleague.url.team";

	static constexpr const wchar_t* ENABLE_LIVE_ALWAYS_TOPMOST = L"feature.show.live.topmost.enable";
	static constexpr const wchar_t* CHAMPION_IMAGE_LOADING_TYPE = L"feature.loading.champion.type";
	static constexpr const wchar_t* GENERAL_POPUP_DIMENSIONS = L"feature.popup.dimensions";

	static constexpr const wchar_t* ENABLE_WHALE_BACKGROUND = L"feature.champselect.show.whalebackground";
	static constexpr const wchar_t* CHAMP_SELECT_WINS_DRAW_TYPE = L"feature.champselect.wins.drawtype";

	static constexpr const wchar_t* ENABLE_SHOW_ORGA_BANNER = L"feature.show.banner.enable";
	static constexpr const wchar_t* ENABLE_SHOW_OCR_MONEY = L"feature.show.live.ocr.enable";
	static constexpr const wchar_t* ENABLE_SUMMONERSRIFT_START_OVERLAY = L"feature.show.live.gamestart.sroverlay";
	static constexpr const wchar_t* ENABLE_SHOW_MONEYDIFFERENCE = L"feature.show.live.moneydiff.enable";
	static constexpr const wchar_t* ENABLE_EVENT_DESCRIPTION = L"feature.show.live.event.description";

	static const std::wstring DEFAULT_FILE_PATH;
	static const std::wstring DEFAULT_FILE_NAME;

	FunctionSettings() : FunctionSettings(DEFAULT_FILE_PATH.c_str()) {}
	FunctionSettings(const wchar_t* filePath) : Settings(filePath) {
		if (setDefaultSettings()) {
			updateSettingsFile();
		}
	}
	virtual ~FunctionSettings() {}
	
};

class OrgaSettings : public Settings {
protected:
	virtual bool setDefaultSettings();
public:
	static constexpr const wchar_t* BLUE_ORGA_ID = L"orga.blue.id";
	static constexpr const wchar_t* RED_ORGA_ID = L"orga.red.id";
	static constexpr const wchar_t* BLUE_ORGA_WINS = L"orga.blue.wins";
	static constexpr const wchar_t* RED_ORGA_WINS = L"orga.red.wins";
	static constexpr const wchar_t* RELOAD_ORGAS = L"orga.reload";

	static const std::wstring DEFAULT_FILE_PATH;
	static const std::wstring DEFAULT_FILE_NAME;

	OrgaSettings() : OrgaSettings(DEFAULT_FILE_PATH.c_str()) {}
	OrgaSettings(const wchar_t* filePath) : Settings(filePath) {
		if (setDefaultSettings()) {
			updateSettingsFile();
		}
	}
	virtual ~OrgaSettings() {}
};

class ResourceDownloaderSettings : public Settings {
protected:
	virtual bool setDefaultSettings();
public:
	static constexpr const wchar_t* RESOURCE_BASE_URL = L"resource.url.base";
	static constexpr const wchar_t* RESOURCE_POSITION_BASE_URL = L"resource.url.base.position";
	static constexpr const wchar_t* RESOURCE_SUMMONERS_BASE_URL = L"resource.url.base.summoners";
	static constexpr const wchar_t* RESOURCE_SHARED_COMPONENTS_BASE_URL = L"resource.url.base.shared";
	static constexpr const wchar_t* FOLDER_NAME = L"resource.folder";
	static constexpr const wchar_t* ICON_URI = L"resource.url.icons.uri";
	static constexpr const wchar_t* TRAITS_URI = L"resource.url.traits.uri";
	static constexpr const wchar_t* ATLAS_URI = L"resource.url.atlas.uri";
	static constexpr const wchar_t* PINGS_URI = L"resource.url.pings.uri";
	static constexpr const wchar_t* SRX_URI = L"resource.url.srx.uri";
	static constexpr const wchar_t* SPECTATOR_URI = L"resource.url.spectator.uri";
	static constexpr const wchar_t* SCOREBOARD_URI = L"resource.url.scoreboard.uri";
	static constexpr const wchar_t* TFT_STAGEICON_URI = L"resource.url.tft.stage.uri";
	static constexpr const wchar_t* EPIC = L"resource.url.spectator.uri";
	static constexpr const wchar_t* EPIC_MONSTER_ATLAS_NAME = L"resource.atlas.epics";
	static constexpr const wchar_t* OBJECTIVES_ATLAS_NAME = L"resource.atlas.objectives";
	static constexpr const wchar_t* GRUBS_BUFF_ICON_NAME = L"resource.icon.grubs.buff";
	static constexpr const wchar_t* CLOUD_DRAKE_ICON_NAME = L"resource.icon.drake.cloud";
	static constexpr const wchar_t* CHEMTECH_DRAKE_ICON_NAME = L"resource.icon.drake.chemtech";
	static constexpr const wchar_t* ELDER_DRAKE_ICON_NAME = L"resource.icon.drake.elder";
	static constexpr const wchar_t* INFERNAL_DRAKE_ICON_NAME = L"resource.icon.drake.infernal";
	static constexpr const wchar_t* HEXTECH_DRAKE_ICON_NAME = L"resource.icon.drake.hextech";
	static constexpr const wchar_t* MOUNTAIN_DRAKE_ICON_NAME = L"resource.icon.drake.mountain";
	static constexpr const wchar_t* OCEAN_DRAKE_ICON_NAME = L"resource.icon.drake.ocean";
	static constexpr const wchar_t* BLUE_TURRET_ICON_NAME = L"resource.icon.turret.blue";
	static constexpr const wchar_t* RED_TURRET_ICON_NAME = L"resource.icon.turret.red";
	static constexpr const wchar_t* RANKED_ICON_UNRANKED = L"resource.shared.icon.ranked.unranked";
	static constexpr const wchar_t* RANKED_ICON_IRON = L"resource.shared.icon.ranked.iron";
	static constexpr const wchar_t* RANKED_ICON_BRONZE = L"resource.shared.icon.ranked.bronze";
	static constexpr const wchar_t* RANKED_ICON_SILVER = L"resource.shared.icon.ranked.silver";
	static constexpr const wchar_t* RANKED_ICON_GOLD = L"resource.shared.icon.ranked.gold";
	static constexpr const wchar_t* RANKED_ICON_PLATIN = L"resource.shared.icon.ranked.platin";
	static constexpr const wchar_t* RANKED_ICON_EMERALD = L"resource.shared.icon.ranked.emerald";
	static constexpr const wchar_t* RANKED_ICON_DIAMOND = L"resource.shared.icon.ranked.diamond";
	static constexpr const wchar_t* RANKED_ICON_MASTER = L"resource.shared.icon.ranked.master";
	static constexpr const wchar_t* RANKED_ICON_GRANDMASTER = L"resource.shared.icon.ranked.grandmaster";
	static constexpr const wchar_t* RANKED_ICON_CHALLENGER = L"resource.shared.icon.ranked.challenger";
	static constexpr const wchar_t* ESPORTS_EPIC_AND_OBJECTIVES_KILLED_ICON = L"resource.spectator.esports";
	static constexpr const wchar_t* SPECTATOR_GOLD_ICON = L"resource.spectator.gold";
	static constexpr const wchar_t* DRAGON_SOUL_SRX_ICON = L"resource.srx.drakes";
	static constexpr const wchar_t* LOW_WINRATE_TRAIT_ICON = L"resource.traits.lowwr";
	static constexpr const wchar_t* HIGH_WINRATE_TRAIT_ICON = L"resource.traits.highwr";
	static constexpr const wchar_t* LOW_MASTERY_TRAIT_ICON = L"resource.traits.champion.lowmastery";
	static constexpr const wchar_t* HIGH_MASTERY_TRAIT_ICON = L"resource.traits.champion.highmastery";
	static constexpr const wchar_t* HIGHEST_CHAMPION_PLAYRATE_ICON = L"resource.traits.champion.highestplayrate";
	static constexpr const wchar_t* HIGH_CHAMPION_PLAYRATE_ICON = L"resource.traits.champion.highplayrate";
	static constexpr const wchar_t* HIGH_DAMAGE_ICON = L"resource.traits.highdmg";
	static constexpr const wchar_t* LOW_LANE_CS_ICON = L"resource.traits.lane.lowcs";
	static constexpr const wchar_t* HIGH_LANE_CS_ICON = L"resource.traits.lane.highcs";
	static constexpr const wchar_t* LOW_LANE_GOLD_DIFF_ICON = L"resource.traits.lane.lowgold";
	static constexpr const wchar_t* HIGH_LANE_GOLD_DIFF_ICON = L"resource.traits.lane.highgold";
	static constexpr const wchar_t* LOW_SURRENDER_RATE_ICON = L"resource.traits.lowsurrender";
	static constexpr const wchar_t* HIGH_SURRENDER_RATE_ICON = L"resource.traits.highsurrender";
	static constexpr const wchar_t* HIGH_ENEMY_SURRENDER_RATE_ICON = L"resource.traits.highenemysurrender";
	static constexpr const wchar_t* LOW_FIRST_BLOOD_RATE_ICON = L"resource.traits.lane.lowfirstblood";
	static constexpr const wchar_t* HIGH_FIRST_BLOOD_RATE_ICON = L"resource.traits.lane.highfirstblood";
	static constexpr const wchar_t* LOW_CHAMPION_WINRATE_ICON = L"resource.traits.champion.lowwinrate";
	static constexpr const wchar_t* HIGH_CHAMPION_WINRATE_ICON = L"resource.traits.champion.highwinrate";

	static constexpr const wchar_t* KDA_TRAIT_ICON = L"resource.traits.kda";
	static constexpr const wchar_t* LOW_DEATHS_TRAIT_ICON = L"resource.traits.lowdeaths";
	static constexpr const wchar_t* PENTAKILLER_TRAIT_ICON = L"resource.traits.penta";
	static constexpr const wchar_t* SOLOKILLER_TRAIT_ICON = L"resource.traits.solokiller";
	static constexpr const wchar_t* LOW_VISION_TRAIT_ICON = L"resource.traits.lowvision";
	static constexpr const wchar_t* HIGH_VISION_TRAIT_ICON = L"resource.traits.highvision";
	static constexpr const wchar_t* TEAMKILLER_ONLY_TRAIT_ICON = L"resource.traits.teamkiller";
	static constexpr const wchar_t* VS_ICON_NAME = L"resource.trait.fight";
	static constexpr const wchar_t* WIN_ICON_NAME = L"resource.trait.winloss";
	static constexpr const wchar_t* WILDCARD_TRAIT_ICON = L"resource.traits.wildcard";
	static constexpr const wchar_t* OVERALL_AVERAGE_TRAIT_ICON = L"resource.traits.overallaverage";
	static constexpr const wchar_t* CS_TRAIT_ICON = L"resource.traits.cs";
	static constexpr const wchar_t* HIGH_KILLS_AND_DEATHS_TRAIT_ICON = L"resource.traits.highkd";
	static constexpr const wchar_t* PLAYER_POSITION_TOPLANER_ICON = L"resource.position.toplaner";
	static constexpr const wchar_t* PLAYER_POSITION_JUNGLER_ICON = L"resource.position.jungler";
	static constexpr const wchar_t* PLAYER_POSITION_MIDLANER_ICON = L"resource.position.midlaner";
	static constexpr const wchar_t* PLAYER_POSITION_BOTLANER_ICON = L"resource.position.botlaner";
	static constexpr const wchar_t* PLAYER_POSITION_SUPPORTER_ICON = L"resource.position.supporter";

	static const std::wstring DEFAULT_FILE_PATH;

	ResourceDownloaderSettings() : ResourceDownloaderSettings(DEFAULT_FILE_PATH.c_str()) {}
	ResourceDownloaderSettings(const wchar_t* filePath) : Settings(filePath) {
		setDefaultSettings();
	}
	virtual ~ResourceDownloaderSettings() {}
};

#endif //__LEAGUE_FUNCTION_SETTINGS__