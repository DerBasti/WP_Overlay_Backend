#ifndef __LEAGUE_TIME_CONVERTER__
#define __LEAGUE_TIME_CONVERTER__

#pragma once
#include <chrono>

class TimeConverter {
public:
	template<class _Duration>
	static std::string AddTimeToCurrent(_Duration added) {
		auto now = std::chrono::system_clock::now();
		now += added;
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		tm timeData;
		localtime_s(&timeData, &in_time_t);
		ss << std::put_time(&timeData, "%H:%M:%S");
		return ss.str();
		
	}
	static std::string CreateDateTimeFromMillis() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		tm timeData;
		localtime_s(&timeData, &in_time_t);
		ss << std::put_time(&timeData, "%Y_%m_%d_-_%H%M%S");
		return ss.str();
	}
};

#endif //__LEAGUE_TIME_CONVERTER__