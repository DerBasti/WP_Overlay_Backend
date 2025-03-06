#ifndef __RIOT_CHAMP_SELECT_ACTION_TYPE__
#define __RIOT_CHAMP_SELECT_ACTION_TYPE__

#include <inttypes.h>

enum class ChampSelectSummonerActionType : uint8_t {
	IDLE,
	PICKING,
	BANNING,
	UNKNOWN,
};

#endif 