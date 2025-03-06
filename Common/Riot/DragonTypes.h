#ifndef __LEAGUE_DRAGON_TYPES__
#define __LEAGUE_DRAGON_TYPES__

#pragma once 

#include <inttypes.h>
#include <string>
#include <unordered_map>
#include <iostream>

enum class DragonType : uint8_t {
	INFERNAL,
	OCEAN,
	MOUNTAIN,
	CLOUD,
	HEXTECH,
	CHEMTECH,
	ELDER,
	UNKNOWN
};

std::ostream& operator<<(std::ostream& out, DragonType dragonType);
std::wostream& operator<<(std::wostream& out, DragonType dragonType);

class DragonTypeStringified {
private:
	DragonTypeStringified() {}
public:
	static DragonType FromString(const std::string& dragonName) {
		const static std::unordered_map<std::string, DragonType> DragonTypesByString{
			{"", DragonType::UNKNOWN},
			{"SRU_Dragon_Hextech", DragonType::HEXTECH},
			{"SRU_Dragon_Chemtech", DragonType::CHEMTECH},
			{"SRU_Dragon_Fire", DragonType::INFERNAL},
			{"SRU_Dragon_Earth", DragonType::MOUNTAIN},
			{"SRU_Dragon_Air", DragonType::CLOUD},
			{"SRU_Dragon_Water", DragonType::OCEAN},
			{"SRU_Dragon_Elder", DragonType::ELDER}
		};
		auto it = DragonTypesByString.find(dragonName);
		return it != DragonTypesByString.cend() ? it->second : DragonType::UNKNOWN;
	}
	static std::string FromType(DragonType type) {
		const static std::unordered_map<DragonType, std::string> DragonTypes{
			{DragonType::UNKNOWN, ""},
			{DragonType::HEXTECH, "SRU_Dragon_Hextech"},
			{DragonType::CHEMTECH, "SRU_Dragon_Chemtech"},
			{DragonType::INFERNAL, "SRU_Dragon_Fire"},
			{DragonType::MOUNTAIN, "SRU_Dragon_Earth"},
			{DragonType::CLOUD, "SRU_Dragon_Air"},
			{DragonType::OCEAN, "SRU_Dragon_Water"},
			{DragonType::ELDER, "SRU_Dragon_Elder"}
		};
		auto it = DragonTypes.find(type);
		return it != DragonTypes.cend() ? it->second : "UNKNOWN";
	}
};

#endif