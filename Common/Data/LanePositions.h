#ifndef __LEAGUE_LANE_POSITIONS__
#define __LEAGUE_LANE_POSITIONS__

#pragma once

#include <inttypes.h>
#include <map>
#include <iostream>

enum class SummonerLanePosition : uint8_t {
	INVALID = (uint8_t)-1,
	TOPLANE = 0,
	JUNGLE,
	MIDLANE,
	BOTLANE,
	SUPPORT,
	POSITION_AMOUNT
};

std::ostream& operator<<(std::ostream& out, const SummonerLanePosition& lane);
std::wostream& operator<<(std::wostream& out, const SummonerLanePosition& lane);

#endif //__LEAGUE_LANE_POSITIONS__