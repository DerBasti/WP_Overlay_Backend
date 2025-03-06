#ifndef __TIME_CYCLED_TASK__
#define __TIME_CYCLED_TASK__

#include <functional>
#include <inttypes.h>
#include "../PerformanceClock.h"

enum class TaskStatus {
	KEEP_RUNNING,
	FINISHED
};

class TimeCycledTask {
private:
	PerformanceClock clock;
	uint64_t currentId;
	uint32_t cycleDurationInMilliseconds;
	TaskStatus lastTaskStatus;
	std::function<TaskStatus()> callback;
public:
	TimeCycledTask();
	TimeCycledTask(uint32_t durationInMillisUntilCallPossible, std::function<TaskStatus()> functionCall);
	virtual ~TimeCycledTask();
	TaskStatus run();
	void reset();
	void setFinished() {
		lastTaskStatus = TaskStatus::FINISHED;
	}
	inline TaskStatus getLastTaskStatus() const {
		return lastTaskStatus;
	}
	inline bool isTaskFinished() const {
		return getLastTaskStatus() == TaskStatus::FINISHED;
	}
};

#endif