#include "LanePositions.h"

std::ostream& operator<<(std::ostream& out, const SummonerLanePosition& lane) {
	static const std::map<SummonerLanePosition, const char*> POSITION_NAMES{
		{SummonerLanePosition::TOPLANE, "TOPLANE"},
		{SummonerLanePosition::JUNGLE, "JUNGLE"},
		{SummonerLanePosition::MIDLANE, "MIDLANE"},
		{SummonerLanePosition::BOTLANE, "BOTLANE"},
		{SummonerLanePosition::SUPPORT, "SUPPORT"},
		{SummonerLanePosition::INVALID, "INVALID"}
	};
	out << POSITION_NAMES.at(lane);
	return out;
};

std::wostream& operator<<(std::wostream& out, const SummonerLanePosition& lane) {
	static const std::map<SummonerLanePosition, const char*> POSITION_NAMES{
		{SummonerLanePosition::TOPLANE, "TOPLANE"},
		{SummonerLanePosition::JUNGLE, "JUNGLE"},
		{SummonerLanePosition::MIDLANE, "MIDLANE"},
		{SummonerLanePosition::BOTLANE, "BOTLANE"},
		{SummonerLanePosition::SUPPORT, "SUPPORT"},
		{SummonerLanePosition::INVALID, "INVALID"}
	};
	out << POSITION_NAMES.at(lane);
	return out;
};
