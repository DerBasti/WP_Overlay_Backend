#ifndef __LEAGUE_OCR_BASE__
#define __LEAGUE_OCR_BASE__

#pragma once

#include <inttypes.h>

class LeagueTeamObjectiveValues {
private:
	float money;
	uint32_t grubs;
public:
	LeagueTeamObjectiveValues() : LeagueTeamObjectiveValues(0.0f, 0) {}
	LeagueTeamObjectiveValues(float money, uint32_t grubs) {
		this->money = money;
		this->grubs = grubs;
	}
	virtual ~LeagueTeamObjectiveValues() {

	}

	inline float getMoney() const {
		return money;
	}
	inline void setMoney(float money) {
		this->money = money;
	}
	inline uint32_t getGrubs() const {
		return grubs;
	}
	inline void setGrubs(uint32_t grubs) {
		this->grubs = grubs;
	}
};

enum class LeagueOCRLocaleDetectionMode : uint8_t {
	OCR_DETECTION_WESTERN_RAYFILL,
	OCR_DETECTION_WESTERN_SPANFLOOD,
	OCR_DETECTION_KOREAN,
};

enum class LetterBoundingBoxColor : uint8_t {
	GREEN,
	BLUE,
	RED,
};

#endif __LEAGUE_OCR_BASE__