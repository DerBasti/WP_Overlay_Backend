#ifndef __LEAGUE_PERFORMANCE_CLOCK__
#define __LEAGUE_PERFORMANCE_CLOCK__

#include <chrono>
#include <inttypes.h>
#include "Logging/Logger.h"

class PerformanceClock {
private:
	uint64_t timestampStart;
	ROSEThreadedLogger logger;
protected:
	virtual void onReset() {

	}
public:
	PerformanceClock() : PerformanceClock("") {}
	PerformanceClock(const char* clockName);
	virtual ~PerformanceClock();

	__inline void reset() {
		timestampStart = GetCurrentMillisecondTimestamp();
		onReset();
	}

	__inline virtual uint64_t getDuration() const {
		return GetCurrentMillisecondTimestamp() - timestampStart;
	}
	static uint64_t GetCurrentMillisecondTimestamp() {
		return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}
};

class StoppablePerformanceClock : public PerformanceClock {
private:
	bool finished;
	uint64_t valueOnFinished;
	uint64_t allowedMaximum;
protected:
	virtual void onReset() {
		finished = false;
		valueOnFinished = 0;
	}
public:
	StoppablePerformanceClock() : StoppablePerformanceClock((std::numeric_limits<uint64_t>::max)()) {
	}
	StoppablePerformanceClock(uint64_t allowedMaximum) {
		this->allowedMaximum = allowedMaximum;
		valueOnFinished = 0;
		finished = false;
	}
	uint64_t stop(bool forceAllowedMaxValue = false) {
		if (!finished) {
			finished = true;
			valueOnFinished = forceAllowedMaxValue ? allowedMaximum : (std::min)(allowedMaximum, PerformanceClock::getDuration());
		}
		return valueOnFinished;
	}
	__inline uint64_t getDuration() const {
		return (finished ? valueOnFinished : PerformanceClock::getDuration());
	}
	inline bool isFinished() const {
		return finished || getDuration() >= allowedMaximum;
	}
};

#endif