#ifndef __LEAGUE_TEAM__
#define __LEAGUE_TEAM__

#include <inttypes.h>

enum class TeamType : uint8_t {
	INVALID = (uint8_t)-1,
	BLUE = 0,
	RED, 
	ORDER = BLUE,
	CHAOS
};

class TeamTypeStringify {
private:
	TeamTypeStringify() {}
public:
	virtual ~TeamTypeStringify() {}
	static const wchar_t* ToStringUnicode(const TeamType type) {
		return type == TeamType::BLUE ? L"BLUE" : (type == TeamType::RED ? L"RED" : L"UNKNOWN/INVALID");
	}
	static const char *ToString(const TeamType type) {
		return type == TeamType::BLUE ? "BLUE" : (type == TeamType::RED ? "RED" : "UNKNOWN/INVALID");
	}
};

#endif 