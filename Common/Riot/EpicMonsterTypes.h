#ifndef __EPIC_MONSTER_TYPES__
#define __EPIC_MONSTER_TYPES__
#pragma once

#include <inttypes.h>
#include <iostream>

enum class EpicMonsterType : uint8_t {
	DRAGON,
	ELDER_DRAGON,
	RIFT_HERALD,
	BARON_NASHOR,
	UNKNOWN,
};

std::ostream& operator<<(std::ostream& out, EpicMonsterType epicMonsterType);
std::wostream& operator<<(std::wostream& out, EpicMonsterType epicMonsterType);

#endif