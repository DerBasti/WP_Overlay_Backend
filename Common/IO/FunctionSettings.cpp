#include "FunctionSettings.h"
#include "../ProjectFilePathHandler.h"
#include "../Logging/ErrorCodeTranslator.h"
#include "../Logging/StackWalker.h"
#include <locale>
#include <codecvt>
#include <algorithm>

const std::wstring FunctionSettings::DEFAULT_FILE_NAME = std::wstring(L"featuresettings.conf");
const std::wstring OrgaSettings::DEFAULT_FILE_NAME = std::wstring(L"orgasettings.conf");

const std::wstring FunctionSettings::DEFAULT_FILE_PATH = ProjectFilePathHandler::GetDefaultFilePathUnicode() + FunctionSettings::DEFAULT_FILE_NAME;
const std::wstring OrgaSettings::DEFAULT_FILE_PATH = ProjectFilePathHandler::GetDefaultFilePathUnicode() + OrgaSettings::DEFAULT_FILE_NAME;
const std::wstring ResourceDownloaderSettings::DEFAULT_FILE_PATH = (ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"icon_download.conf"));

#pragma warning(disable:4996)
Settings::Settings(const wchar_t* filePath) {
	this->lastError = "";
	this->settingsFilePath = std::wstring(filePath);
	reloadSettingsFile();
}
Settings::~Settings() {

}

void Settings::readSettingsFromFile(FILE* fh) {
	wchar_t buffer[0x400] = { 0x00 };
	while (fgetws(buffer, 0x400, fh) != nullptr) {
		std::wstring keyValuePair = std::wstring(buffer);
		if (keyValuePair.length() < 3 || keyValuePair.at(0) == '#') {
			continue;
		}
		std::wstring key = keyValuePair.substr(0, keyValuePair.find('='));
		std::wstring value = keyValuePair.substr(key.length() + 1);
		while (value.length() > 0 && (value.back() == '\r' || value.back() == '\n')) {
			value.pop_back();
		}
		logger.logDebug("Key: ", key.c_str(), " | Value: ", value.c_str());
		setValue(key, value.c_str());
	}
}

void Settings::updateSettingsFile() {
	FILE* fh = nullptr;
	for (uint32_t i = 0;!fh && i < 10; i++) {
		fh = _wfsopen(settingsFilePath.c_str(), L"wb+", _SH_SECURE);
		if (!fh) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
	if (!fh) {
		logger.logWarn("Couldn't update settings file at path: ", settingsFilePath.c_str(), ". Error reason: ", ErrorCodeTranslator::GetErrorCodeString().c_str());
		return;
	}
	std::wstring resultString = std::wstring();
	logger.logDebug("Saving current state of settings file at: ", settingsFilePath.c_str(), ". ");
	for (auto keyValuePair : settingsMap) {
		if (commentsForSettings.find(keyValuePair.first) != commentsForSettings.cend()) {
			resultString += std::wstring(L"#") + commentsForSettings.at(keyValuePair.first.c_str()) + std::wstring(L"\n");
		}
		resultString += keyValuePair.first + std::wstring(L"=") + keyValuePair.second + std::wstring(L"\n");
	}
	fwrite(resultString.c_str(), sizeof(wchar_t), resultString.length(), fh);
	fputwc(0x00, fh);
	fclose(fh);
}

void Settings::reloadSettingsFile() {
	std::function<bool()> retryRead = [&]() {
		FILE* fh = _wfsopen(settingsFilePath.c_str(), L"rb", _SH_SECURE);
		bool success = (fh != nullptr);
		if (fh) {
			uint64_t fileSize = 0;
			_fseeki64(fh, 0, SEEK_END);
			fileSize = _ftelli64(fh);
			rewind(fh);
			lastError = ErrorCodeTranslator::GetErrorCodeString();
			readSettingsFromFile(fh);
			fclose(fh);
		}
		else {
			lastError = ErrorCodeTranslator::GetErrorCodeString();
		}
		return success;
	};
	bool successfulRead = false;
	for (uint32_t i = 0; i < 10; i++) {
		if (retryRead()) {
			successfulRead = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

std::wstring Settings::getValue(std::wstring key, const wchar_t* defaultValue) const {
	std::shared_lock<std::shared_mutex> guard(settingsMapMutex);
	auto it = settingsMap.find(key);
	if (it != settingsMap.cend()) {
		return it->second;
	}
	return defaultValue == nullptr ? std::wstring(L"") : std::wstring(defaultValue);
}

std::string Settings::getValueUtf8(std::wstring key, const char* defaultValue) const {
	std::shared_lock<std::shared_mutex> guard(settingsMapMutex);
	std::wstring value = getValue(key, L"");
	if (value.empty()) {
		return std::string(defaultValue);
	}
	std::string convertedValue;
	std::transform(value.begin(), value.end(), std::back_inserter(convertedValue), [](wchar_t c) {
		return (char)c;
	});
	return convertedValue;
}

void Settings::setValue(std::wstring key, const wchar_t* value) {
	std::wstring previousValue;
	{
		std::unique_lock<std::shared_mutex> guard(settingsMapMutex);
		auto currentKeyValue = settingsMap.find(key);
		previousValue = currentKeyValue != settingsMap.cend() ? currentKeyValue->second : std::wstring();
		settingsMap.insert_or_assign(key, std::move(std::wstring(value)));
	}
	if (_wcsicmp(previousValue.c_str(), value) != 0) {
		std::wstring valueAsStr = std::wstring(value);
		notifyListenerForKey(key, valueAsStr);
	}
}


void Settings::addListenerForKey(std::wstring keyToListenFor, SettingsKeyChangedListener* listener) {
	listenedKeysMap.insert(std::make_pair(keyToListenFor, listener));
	listenerMap.insert(std::make_pair(listener, keyToListenFor));
}

void Settings::removeAllListenerForCaller(SettingsKeyChangedListener* listenerToRemove) {
	std::vector<std::wstring> keysToLookFor;
	if (listenerMap.find(listenerToRemove) != listenerMap.cend()) {
		auto it = listenerMap.equal_range(listenerToRemove);
		for (auto i = it.first; i != it.second; ++i) {
			keysToLookFor.push_back(i->second);
		}
	}
	listenerMap.erase(listenerToRemove);
	for(auto key : keysToLookFor) {
		auto it = listenedKeysMap.equal_range(key);
		for (auto i = it.first; i != it.second; ++i) {
			if (i->second == listenerToRemove) {
				listenedKeysMap.erase(i);
				break;
			}
		}
	}
}

void Settings::addCommentForKey(std::wstring key, std::wstring comment) {
	if (!comment.empty()) {
		commentsForSettings.insert_or_assign(key, comment);
	}
	else if (commentsForSettings.find(key) != commentsForSettings.cend()) {
		commentsForSettings.erase(key);
	}
}

bool Settings::setValueIfAbsent(std::wstring key, const wchar_t* value, std::wstring comment) {
	bool wasAbsent = isValueAbsent(key);
	if (wasAbsent) {
		addCommentForKey(key, comment);
		setValue(key, value);
	}
	return wasAbsent;
}

void Settings::notifyListenerForKey(const std::wstring& key, const std::wstring& value) const {
	if (listenedKeysMap.find(key) != listenedKeysMap.cend()) {
		logger.logDebug("Notifying listeners of settings key: ", key.c_str(), " of updated value (", value.c_str(), ")");
		auto iteratorRange = listenedKeysMap.equal_range(key);
		for(auto i=iteratorRange.first; i != iteratorRange.second;++i) {
			i->second->onSettingsKeyValueChanged(key, value);
		}
	}
}

SettingsKeyChangedListener::SettingsKeyChangedListener() {

}

SettingsKeyChangedListener::~SettingsKeyChangedListener() {
	for (auto settings : settingsListenedTo) {
		settings->removeAllListenerForCaller(this);
	}
}

void SettingsKeyChangedListener::onSettingsKeyValueChanged(const std::wstring& key, const std::wstring& newValue) {
	if (keysToListenFor.find(key) != keysToListenFor.cend()) {
		keysToListenFor.at(key)(newValue);
	}
}

void SettingsKeyChangedListener::addKeyToListenFor(Settings* settings, const std::wstring& key, std::function<void(const std::wstring&)> callback) {
	settingsListenedTo.insert(settings);
	keysToListenFor.insert_or_assign(key, callback);
	settings->addListenerForKey(key, this);
}


bool FunctionSettings::setDefaultSettings() {
	bool wasAbsent = setValueIfAbsent(FunctionSettings::ENABLE_LIVE_ALWAYS_TOPMOST, L"0");
	wasAbsent |= setValueIfAbsent(FunctionSettings::ENABLE_SHOW_ORGA_BANNER, L"1");
	wasAbsent |= setValueIfAbsent(FunctionSettings::LATEST_PATCH_VERSION, L"");
	wasAbsent |= setValueIfAbsent(FunctionSettings::ENABLE_SUMMONERSRIFT_START_OVERLAY, L"1");
	wasAbsent |= setValueIfAbsent(FunctionSettings::ENABLE_SHOW_MONEYDIFFERENCE, L"1");
	wasAbsent |= setValueIfAbsent(FunctionSettings::CHAMPION_IMAGE_LOADING_TYPE, L"LAZY", L"Two options: LAZY = 'Load if necessary on request', EAGER = 'Load everything on startup'. Always evaluates to LAZY if not explicitly set to 'EAGER'");
	wasAbsent |= setValueIfAbsent(FunctionSettings::ENABLE_SHOW_OCR_MONEY, L"1");
	wasAbsent |= setValueIfAbsent(FunctionSettings::ENABLE_EVENT_DESCRIPTION, L"0");
	wasAbsent |= setValueIfAbsent(FunctionSettings::BANNER_GAME_EVENT_NAME, L"");
	wasAbsent |= setValueIfAbsent(FunctionSettings::ENABLE_WHALE_BACKGROUND, L"1");
	wasAbsent |= setValueIfAbsent(FunctionSettings::BANNER_GAME_EVENT_DESCRIPTION, L"");
	wasAbsent |= setValueIfAbsent(FunctionSettings::GAME_AMOUNT_TO_PLAY_IN_SESSION, L"2");
	wasAbsent |= setValueIfAbsent(FunctionSettings::PRIMELEAGUE_URL_STARTER_TOKEN, L"https://www.primeleague.gg/de/leagues/prm/");
	wasAbsent |= setValueIfAbsent(FunctionSettings::CHAMP_SELECT_WINS_DRAW_TYPE, L"LITERAL", L"Can be either LITERAL for numeric expression or DOT for dot-representation below team icon"); 
	return wasAbsent;
}

bool OrgaSettings::setDefaultSettings() {
	bool wasAbsent = setValueIfAbsent(OrgaSettings::BLUE_ORGA_ID, L"");
	wasAbsent |= setValueIfAbsent(OrgaSettings::BLUE_ORGA_WINS, L"");
	wasAbsent |= setValueIfAbsent(OrgaSettings::RED_ORGA_ID, L"");
	wasAbsent |= setValueIfAbsent(OrgaSettings::RED_ORGA_WINS, L"");
	wasAbsent |= setValueIfAbsent(OrgaSettings::RELOAD_ORGAS, L"1");
	return wasAbsent;
}

bool ResourceDownloaderSettings::setDefaultSettings() {
	bool wasAbsent = setValueIfAbsent(ResourceDownloaderSettings::RESOURCE_BASE_URL, L"https://raw.communitydragon.org/latest/game/assets/ux/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RESOURCE_POSITION_BASE_URL, L"https://raw.communitydragon.org/latest/plugins/rcp-fe-lol-clash/global/default/assets/images/position-selector/positions/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RESOURCE_SUMMONERS_BASE_URL, L"http://ddragon.leagueoflegends.com/cdn/%s.1/img/spell/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RESOURCE_SHARED_COMPONENTS_BASE_URL, L"https://raw.communitydragon.org/latest/plugins/rcp-fe-lol-shared-components/global/default/%s");

	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::TRAITS_URI, L"traiticons/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::SRX_URI, L"srx/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::ICON_URI, L"minimap/icons/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::ATLAS_URI, L"lol/%s"); 
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::FOLDER_NAME, L"icons/");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::PINGS_URI, L"minimap/pings/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::SCOREBOARD_URI, L"scoreboard/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::SPECTATOR_URI, L"spectator/%s");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::TFT_STAGEICON_URI, L"tft/stageicons/%s");

	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_UNRANKED, L"unranked.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_IRON, L"iron.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_BRONZE, L"bronze.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_SILVER, L"silver.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_GOLD, L"gold.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_PLATIN, L"platinum.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_EMERALD, L"emerald.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_DIAMOND, L"diamond.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_MASTER, L"master.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_GRANDMASTER, L"grandmaster.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RANKED_ICON_CHALLENGER, L"challenger.png");

	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::DRAGON_SOUL_SRX_ICON, L"dragonuiprototype.igr_release.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::EPIC_MONSTER_ATLAS_NAME, L"objectivealignment_atlas.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::OBJECTIVES_ATLAS_NAME, L"right_icons_grub.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::GRUBS_BUFF_ICON_NAME, L"touchofthevoid.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::CLOUD_DRAKE_ICON_NAME, L"dragonairminimap.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::CHEMTECH_DRAKE_ICON_NAME, L"dragonchemtechmini.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::ELDER_DRAKE_ICON_NAME, L"dragonelderminimap.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HEXTECH_DRAKE_ICON_NAME, L"dragonhextechminimap.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::INFERNAL_DRAKE_ICON_NAME, L"dragonfireminimap.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::MOUNTAIN_DRAKE_ICON_NAME, L"dragonearthminimap.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::OCEAN_DRAKE_ICON_NAME, L"dragonwaterminimap.png");

	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::BLUE_TURRET_ICON_NAME, L"icon_ui_tower_minimap_order_bounty_ship.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::RED_TURRET_ICON_NAME, L"icon_ui_tower_minimap_chaos_bounty_ship.png");

	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::SPECTATOR_GOLD_ICON, L"replayatlas.playerstats_iconfix.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::ESPORTS_EPIC_AND_OBJECTIVES_KILLED_ICON, L"scoreboardatlas.igr_release.png");

	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_DAMAGE_ICON, L"trait_icon_3_demolitionist.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_LANE_CS_ICON, L"trait_icon_guardian.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_LANE_CS_ICON, L"trait_icon_3_paragon.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_LANE_GOLD_DIFF_ICON, L"trait_icon_gunslinger.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_LANE_GOLD_DIFF_ICON, L"trait_icon_phantom.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_KILLS_AND_DEATHS_TRAIT_ICON, L"trait_icon_5_cannoneer.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::PENTAKILLER_TRAIT_ICON, L"trait_icon_6_colossus.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_WINRATE_TRAIT_ICON, L"trait_icon_6_innovator.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::WILDCARD_TRAIT_ICON, L"trait_icon_6_mercenary.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::KDA_TRAIT_ICON, L"trait_icon_6_striker.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::CS_TRAIT_ICON, L"trait_icon_6_syndicate.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_VISION_TRAIT_ICON, L"trait_icon_8_corrupted.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_DEATHS_TRAIT_ICON, L"trait_icon_8_heart.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::TEAMKILLER_ONLY_TRAIT_ICON, L"trait_icon_8_parallel.tft_set8_stage2.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_VISION_TRAIT_ICON, L"trait_icon_8_recon.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::SOLOKILLER_TRAIT_ICON, L"trait_icon_8_renegade.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_MASTERY_TRAIT_ICON, L"trait_icon_elementalist.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_MASTERY_TRAIT_ICON, L"trait_icon_glacial.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::OVERALL_AVERAGE_TRAIT_ICON, L"trait_icon_shapeshifter.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_WINRATE_TRAIT_ICON, L"trait_icon_imperial.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_SURRENDER_RATE_ICON, L"trait_icon_8_threat.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_SURRENDER_RATE_ICON, L"trait_icon_3_vanguard.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_ENEMY_SURRENDER_RATE_ICON, L"trait_icon_12_eldritch.tft_set12.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_FIRST_BLOOD_RATE_ICON, L"trait_icon_3_battlecast.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_FIRST_BLOOD_RATE_ICON, L"trait_icon_12_time.tft_set12.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_CHAMPION_PLAYRATE_ICON, L"trait_icon_6_sniper.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGHEST_CHAMPION_PLAYRATE_ICON, L"trait_icon_noble.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::LOW_CHAMPION_WINRATE_ICON, L"trait_icon_phantom.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::HIGH_CHAMPION_WINRATE_ICON, L"trait_icon_3_starguardian.png");
	

	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::VS_ICON_NAME, L"all_in.png"); 
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::WIN_ICON_NAME, L"win_loss_icon.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::PLAYER_POSITION_TOPLANER_ICON, L"icon-position-top.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::PLAYER_POSITION_JUNGLER_ICON, L"icon-position-jungle.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::PLAYER_POSITION_MIDLANER_ICON, L"icon-position-middle.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::PLAYER_POSITION_BOTLANER_ICON, L"icon-position-bottom.png");
	wasAbsent |= setValueIfAbsent(ResourceDownloaderSettings::PLAYER_POSITION_SUPPORTER_ICON, L"icon-position-utility.png");

	return wasAbsent;
}

