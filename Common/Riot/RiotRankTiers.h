#ifndef __RIOT_RANKED_TIER__
#define __RIOT_RANKED_TIER__

#include <inttypes.h>
#include <unordered_map>
#include <string>

class GUIColor;

enum class RankedTier {
	UNRANKED,
	IRON,
	BRONZE,
	SILVER,
	GOLD,
	PLATINUM,
	EMERALD,
	DIAMOND,
	MASTER,
	GRANDMASTER,
	CHALLENGER,
	UNKNOWN
};

class RiotRanking {
private:
	RankedTier tier;
	uint8_t division;
	int16_t lp;
public:
	static const GUIColor RankColors[];
	RiotRanking();
	RiotRanking(RankedTier tier, uint8_t div, int16_t lp);
	virtual ~RiotRanking();
	static uint8_t FromStringToDivision(const char* str);
	static std::wstring ToStringFromRankedTier(RankedTier rankedTier);
	std::wstring toUnicodeString() const;

	inline RankedTier getTier() const {
		return tier;
	}
	inline uint8_t getDivision() const {
		return division;
	}
	inline int16_t getLp() const {
		return lp;
	}
	RiotRanking& operator=(const RiotRanking& other) = default;
};

#endif //__RIOT_RANKED_TIER__