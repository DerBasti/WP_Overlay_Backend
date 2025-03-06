#include "TimeCycledTask.h"

std::function<TaskStatus()> EMPTY_FUNCTION = []() { return TaskStatus::FINISHED; };

TimeCycledTask::TimeCycledTask() : TimeCycledTask(1u, EMPTY_FUNCTION) {

}

TimeCycledTask::TimeCycledTask(uint32_t durationInMillisUntilCallPossible, std::function<TaskStatus()> functionCall) {
	currentId = 0;
	lastTaskStatus = TaskStatus::KEEP_RUNNING;
	cycleDurationInMilliseconds = (std::max)(1u, durationInMillisUntilCallPossible);
	this->callback = functionCall;
}

TimeCycledTask::~TimeCycledTask() {

}

TaskStatus TimeCycledTask::run() {
	uint64_t id = clock.getDuration() / cycleDurationInMilliseconds;
	if (lastTaskStatus  == TaskStatus::FINISHED || currentId == id) {
		return lastTaskStatus;
	}
	currentId = id;
	lastTaskStatus = callback();
	return lastTaskStatus;
}

void TimeCycledTask::reset() {
	currentId = 0;
	lastTaskStatus = TaskStatus::KEEP_RUNNING;
	clock.reset();
}