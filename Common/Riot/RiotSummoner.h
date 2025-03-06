#ifndef __RIOT_SUMMONER__
#define __RIOT_SUMMONER__
#pragma once

#include <inttypes.h>
#include "../Logging/Logger.h"
#include "./RiotRankTiers.h"

class RiotSummoner {
private:
	std::string puuid;
	uint64_t accountId;
	uint64_t summonerId;
	std::string accountNameUnicode;
	std::wstring encodedAccountName;
	ROSEThreadedLogger logger;
	RiotRanking rank;
public:
	RiotSummoner(std::string puuid, uint64_t accountId, uint64_t summonerId, std::string unicodeName);
	virtual ~RiotSummoner();

	inline std::string getPuuid() const {
		return puuid;
	}
	inline uint64_t getAccountId() const {
		return accountId;
	}
	inline uint64_t getSummonerId() const {
		return summonerId;
	}
	inline std::string getAccountName() const {
		return accountNameUnicode;
	}
	inline std::wstring getAccountNameEncoded() const {
		return encodedAccountName;
	}
	inline RiotRanking getRank() const {
		return rank;
	}
	inline void setRank(RiotRanking newRank) {
		rank = newRank;
	}
};

#endif //__RIOT_SUMMONER__