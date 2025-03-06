#include "EpicMonsterTypes.h"
#include <unordered_map>

std::ostream& operator<<(std::ostream& out, EpicMonsterType epicMonsterType) {
	static std::unordered_map<EpicMonsterType, const char*> STRING_MAP{
		{EpicMonsterType::BARON_NASHOR, "Baron Nashor"},
		{EpicMonsterType::RIFT_HERALD, "Rift Herald"},
		{EpicMonsterType::DRAGON, "Dragon"},
		{EpicMonsterType::ELDER_DRAGON, "Elder Dragon"},
		{EpicMonsterType::UNKNOWN,"UNKNOWN"},
	};
	out << STRING_MAP.at(epicMonsterType);
	return out;
}

std::wostream& operator<<(std::wostream& out, EpicMonsterType epicMonsterType) {
	static std::unordered_map<EpicMonsterType, const wchar_t*> STRING_MAP{
	{EpicMonsterType::BARON_NASHOR, L"Baron Nashor"},
	{EpicMonsterType::RIFT_HERALD, L"Rift Herald"},
	{EpicMonsterType::DRAGON, L"Dragon"},
	{EpicMonsterType::ELDER_DRAGON, L"Elder Dragon"},
	{EpicMonsterType::UNKNOWN, L"UNKNOWN"},
	};
	out << STRING_MAP.at(epicMonsterType);
	return out;
}