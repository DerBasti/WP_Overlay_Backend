#ifndef __LEAGUE_SUMMONER_SPELL__
#define __LEAGUE_SUMMONER_SPELL__

#include <unordered_map>
#include <inttypes.h>
#include <algorithm>
#include <cstring>
#include "../../Common/ProjectFilePathHandler.h"
#include "json/document.h"

class SummonerSpell {
private:
	const static constexpr char* SUMMONER_JSON_URL = "http://ddragon.leagueoflegends.com/cdn/%s/data/en_US/summoner.json";

	static std::unordered_map<std::wstring, SummonerSpell*> SummonerSpellsByName;
	static std::unordered_map<uint64_t, SummonerSpell*> SummonerSpellsById;
	uint32_t id;
	std::wstring spellName;
	std::wstring imagePath;
	SummonerSpell(uint32_t id, const wchar_t* name, const wchar_t* imagePath) {
		this->id = id;
		this->spellName = std::wstring(name);
		this->imagePath = std::wstring(imagePath);
	}
	static std::string GetLatestJsonUrl();
	static rapidjson::Document GetData();
public:
	static void Init();
	static void OnDestroy() {
		for (auto it : SummonerSpellsById) {
			delete it.second;
		}
		SummonerSpellsByName.clear();
		SummonerSpellsById.clear();
	}
	__inline static SummonerSpell* GetSpellByName(const char* name) {
		std::string temp = std::string(name);
		std::wstring summoner = std::wstring(temp.c_str(), temp.c_str() + temp.length());
		return GetSpellByName(summoner.c_str());
	}
	__inline static SummonerSpell* GetSpellByName(const wchar_t* name) {
		return SummonerSpellsByName.find(std::wstring(name)) != SummonerSpellsByName.cend() ? SummonerSpellsByName.at(std::wstring(name)) : nullptr;
	}
	__inline static SummonerSpell* GetSpellById(uint64_t id) {
		return SummonerSpellsById.find(id) != SummonerSpellsById.cend() ? SummonerSpellsById.at(id) : nullptr;
	}
	__inline static const std::unordered_map<uint64_t, SummonerSpell*>& GetSpellsSortedById() {
		return SummonerSpellsById;
	}

	virtual ~SummonerSpell() {

	}
	__inline uint32_t getId() const {
		return id;
	}
	__inline std::string getSpellName() const {
		std::string str;
		std::transform(spellName.begin(), spellName.end(), std::back_inserter(str), [](wchar_t c) {
			return (char)c;
		});
		return str;
	}
	__inline std::wstring getSpellNameUnicode() const {
		return spellName;
	}
	__inline std::string getImagePath() const {
		std::string str;
		std::transform(imagePath.begin(), imagePath.end(), std::back_inserter(str), [](wchar_t c) {
			return (char)c;
		});
		return str;
	}
	std::wstring getImagePathUnicode() const {
		return imagePath;
	}
};

#endif