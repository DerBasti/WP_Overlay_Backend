#include "ThreadHandler.h"
#include "../Logging/CrashHandler.h"
#include <Windows.h>

ThreadHandler* ThreadHandler::instance = nullptr;

ThreadHandler::ThreadHandler() {
	runningFlag = true;
	cleanupFinished = false;
	threadPool = new BS::thread_pool(6);
	threadPool->set_rethrow_exceptions(true);
	cleanupThread = threadPool->submit_task_void([&]() {
		CrashHandler::AttachSignalHandler();
		while (runningFlag) {
			if (!threadCache.empty()) {
				std::unique_lock<std::shared_mutex> lock(threadCacheMutex);
				for (auto it = threadCache.begin(); it != threadCache.end(); it++) {
					auto threadClient = it;
					auto futureStatus = threadClient->wait_for(std::chrono::nanoseconds(1));
					if (futureStatus == std::future_status::ready) {
						threadClient->get();
						it = threadCache.erase(it);
						if (it == threadCache.end()) {
							break;
						}
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	});
}

ThreadHandler::~ThreadHandler() {
	stopAllThreads();
}

#ifdef INCLUDE_SOURCE_LOCATION
void ThreadHandler::appendAsThread(std::function<ThreadHandlerTaskStatus()> function, uint32_t sleepTimeInMillis, std::function<void()> onFinished, const std::source_location location) {
	std::string callerFunctionFileName = std::string(location.file_name());
	uint32_t callerFunctionLine = location.line();
	std::string callerFunctionName = std::string(location.function_name());
#else
void ThreadHandler::appendAsThread(std::function<ThreadHandlerTaskStatus()> function, uint32_t sleepTimeInMillis, std::function<void()> onFinished) {
	std::string callerFunctionFileName = std::string();
	uint32_t callerFunctionLine = 0;
	std::string callerFunctionName = std::string();
#endif
	std::future<void> task = threadPool->submit_task_void([function, location, sleepTimeInMillis, onFinished, callerFunctionFileName, callerFunctionLine, callerFunctionName, this]() {
		bool continueRunning = true;
		bool exceptionThrown = false;
		CoInitialize(nullptr);
		CrashHandler::AttachSignalHandler(); 
#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
		//__try {
		try{
			do {
				continueRunning = (function() == ThreadHandlerTaskStatus::CONTINUE_RUNNING ? true : false);
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimeInMillis));
			} while (runningFlag && continueRunning);
			onFinished();
		} catch(...) {
		//__except (EXCEPTION_EXECUTE_HANDLER) {
			logger.logError("Exception occured in subthread. No trace data will be loggable. Possible Caller (if not empty): [", callerFunctionFileName.c_str(), "(", callerFunctionLine, "): ", callerFunctionName.c_str(), "], Trace - Hint: SleepTime of ", sleepTimeInMillis, "ms.");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			CrashHandler::HandleTermination(location);
			exceptionThrown = true;
			std::rethrow_exception(std::current_exception());
		}
		CoUninitialize();
	});
	std::unique_lock<std::shared_mutex> lock(threadCacheMutex);
	threadCache.push_back(std::move(task));
}

void ThreadHandler::stopAllThreads() {
	runningFlag = false;
	if (cleanupThread.valid()) {
		cleanupThread.get();
	}
	threadCache.clear();
	cleanupFinished = true;
}