#ifndef __LEAGUE_INGAME_OBJECT_TYPE__
#define __LEAGUE_INGAME_OBJECT_TYPE__
#pragma once

#include <inttypes.h>

enum class LeagueIngameObjectType : uint8_t {
	UNKNOWN,
	CHAMPION,
	NEXT_DRAGON_INDICATOR,
	DRAGON,
	RIFT_HERALD,
	BARON,
	TURRET,
	INHIBITOR
};

#endif //__LEAGUE_INGAME_OBJECT_TYPE__