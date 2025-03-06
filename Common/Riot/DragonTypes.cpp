#include "DragonTypes.h"

static std::unordered_map<DragonType, const char*> STRING_MAP{
	{DragonType::CHEMTECH, "Chemtech"},
	{DragonType::CLOUD, "Cloud"},
	{DragonType::ELDER, "Elder"},
	{DragonType::HEXTECH, "Hextech"},
	{DragonType::INFERNAL, "Infernal"},
	{DragonType::MOUNTAIN, "Mountain"},
	{DragonType::OCEAN, "Ocean"},
	{DragonType::UNKNOWN, "Unknown"},
};

std::ostream& operator<<(std::ostream& out, DragonType dragonType) {
	out << STRING_MAP.at(dragonType);
	return out;
}
std::wostream& operator<<(std::wostream& out, DragonType dragonType) {
	out << STRING_MAP.at(dragonType);
	return out;
}