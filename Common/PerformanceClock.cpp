#include "PerformanceClock.h"

PerformanceClock::PerformanceClock(const char* clockName) {
	logger.setLoggerName(clockName);
	logger.setLogFileOutput(SynchronizedLogFileFactory::GetDefaultInstance());
	timestampStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
PerformanceClock::~PerformanceClock() {
	//logger.logDebug("Duration was: ", getDuration(), "ms.");
}