#ifndef __LEAGUE_THREAD_HANDLER__
#define __LEAGUE_THREAD_HANDLER__

#if _MSVC_LANG >= 202002L
#define INCLUDE_SOURCE_LOCATION
#endif

#include <thread>
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <future>
#include <shared_mutex>
#include "../Logging/Logger.h"
#include "../Logging/CrashHandler.h"
#include "ThreadPool.hpp"

#ifdef INCLUDE_SOURCE_LOCATION
#include <source_location>
#endif

enum class ThreadHandlerTaskStatus : bool {
	RUNNING_FINISHED,
	CONTINUE_RUNNING,
};

class ThreadHandlerChildProcess {
private:
	std::thread runnerThread;
	uint32_t sleepTimePerCycle;
	std::function<void()> onFinishFunction;
	std::function<ThreadHandlerTaskStatus()> mainFunction;
public:
	ThreadHandlerChildProcess(std::function<ThreadHandlerTaskStatus()> functionCall, uint32_t sleepTimeInMillis, std::function<void()> onDestroy = []() {}) {
		this->mainFunction = functionCall;
		this->sleepTimePerCycle = sleepTimeInMillis;
		this->onFinishFunction = onDestroy;
	}
	virtual ~ThreadHandlerChildProcess() {

	}
	__inline std::thread& getThread() {
		return runnerThread;
	}
	__inline void setThread(std::thread&& thread) {
		this->runnerThread = std::move(thread);
	}
};

class ThreadHandler {
private:
	BS::thread_pool* threadPool;
	std::future<void> cleanupThread;
	std::shared_mutex threadCacheMutex;
	std::vector<std::future<void>> threadCache;
	bool runningFlag;
	bool cleanupFinished;
	ROSEThreadedLogger logger;
	static ThreadHandler* instance;
	ThreadHandler();
public:
	virtual ~ThreadHandler();

	__inline static ThreadHandler* getInstance() {
		if (instance == nullptr) {
			instance = new ThreadHandler();
		}
		return instance;
	}

	inline static void onShutdown() {
		delete instance;
		instance = nullptr;
	}

#ifdef INCLUDE_SOURCE_LOCATION
	void appendAsThread(std::function<ThreadHandlerTaskStatus()> functionCall, uint32_t sleepTimeInMillis, std::function<void()> onDestroy = []() {}, const std::source_location location = std::source_location::current());
#else
	void appendAsThread(std::function<ThreadHandlerTaskStatus()> functionCall, uint32_t sleepTimeInMillis, std::function<void()> onDestroy = []() {});
#endif
	template<class _Type>
	std::future<_Type> createAsyncTask(std::function<_Type()> functionCall) {
		auto result = std::async(std::launch::async, [](std::function<_Type()> callback) {
			CrashHandler::AttachSignalHandler();
			return callback();
		}, functionCall);
		return result;
	}

	void stopAllThreads();
	
	__inline bool isRunningAllowed() const {
		return runningFlag;
	}
	__inline bool isCleanupFinished() const {
		return cleanupFinished;
	}
};

template<class _Data>
class ThreadDataListener {
private:
	std::mutex listenerMutex;
	std::vector<std::function<void(std::shared_ptr<_Data>)>> listener;
public:
	ThreadDataListener() {

	}
	virtual ~ThreadDataListener() {
	
	}
	__inline void provideData(std::shared_ptr<_Data> data) {
		std::lock_guard<std::mutex> guard(listenerMutex);
		for (auto it : listener) {
			it(data);
		}
	}
	__inline void addListenerFunction(std::function<void(std::shared_ptr<_Data>)> listenerFunction) {
		std::lock_guard<std::mutex> guard(listenerMutex);
		this->listener.push_back(listenerFunction);
	}

	__inline void clearListeners() {
		std::lock_guard<std::mutex> guard(listenerMutex);
		listener.clear();
	}
};

#endif //__LEAGUE_THREAD_HANDLER__