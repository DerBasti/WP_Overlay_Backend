#ifndef __ROSE_LOGGER__
#define __ROSE_LOGGER__

#if _MSVC_LANG >= 202002
	#define USE_SOURCE_LOCATION
	#include <source_location>
#endif

#include <inttypes.h>
#include <iostream>
#include <time.h>
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <chrono>
#include <mutex>
#include <thread>
#include <ctime>
#include <chrono>
#include <queue>
#include <fstream>
#include <atomic>
#include <unordered_map>

enum class LogLevelTextColorSchema : uint8_t {
	DEFAULT = 7,
	GREEN = 10,
	BLUE = 11,
	RED = 12,
	ORANGE = 14,
	WHITE = 15
};

class LogLevel {
private:
	const char* name;
	uint8_t importance;
	uint8_t colorScheme;
	constexpr LogLevel(const char* name, uint8_t urgency, const LogLevelTextColorSchema& colorSchema) : LogLevel(name, urgency, static_cast<uint8_t>(colorSchema)) {}
	constexpr LogLevel(const char* name, uint8_t urgency, const uint8_t colorSchema) : name(name), importance(urgency), colorScheme(colorSchema) {}
public:
	const static LogLevel* TRACE;
#ifdef DEBUG
#define _OTHER_DEBUG DEBUG
#undef DEBUG
#endif
	const static LogLevel* DEBUG;
	const static LogLevel* INFO;
	const static LogLevel* WARNING;
#ifdef ERROR
#define _OTHER_ERROR ERROR
#undef ERROR
#endif
	const static LogLevel* ERROR;
	const static LogLevel* DEFAULT_LOGLEVEL;

	virtual ~LogLevel() {

	}
	__inline bool operator==(const LogLevel& other) const {
		return getImportance() == other.getImportance();
	}
	__inline bool operator!=(const LogLevel& other) const {
		return !(this->operator==(other));
	}
	__inline bool operator<(const LogLevel& other) const {
		return getImportance() < other.getImportance();
	}
	__inline const char* getName() const {
		return name;
	}
	__inline uint8_t getImportance() const {
		return importance;
	}
	__inline uint8_t getColorScheme() const {
		return colorScheme;
	}
};

template<class S>
struct is_string_type : std::false_type {};
template<> struct is_string_type<std::string> : std::true_type { using type = std::string; };
template<> struct is_string_type<std::wstring> : std::true_type { using type = std::wstring; };

template<class S>
struct is_char_type : std::false_type {};
template<> struct is_char_type<char> : std::true_type { using type = char; };
template<> struct is_char_type<wchar_t> : std::true_type { using type = wchar_t; };

class SynchronizedLogFile {
private:
	std::wstring filePath;
	std::mutex lockMutex;
public:
	SynchronizedLogFile(std::wstring filePath) : SynchronizedLogFile(filePath, true) {

	}
	SynchronizedLogFile(std::wstring filePath, bool clearFile);
	~SynchronizedLogFile() {

	}
	void clear() {
		std::lock_guard<std::mutex> guard(lockMutex);
		std::wofstream log(filePath.c_str(), std::wofstream::ate);
		log.close();
	}
	__inline void write(const std::wstringstream& output) {
		write<std::wstring>(output.str());
	}

	template<class StringType, class = typename std::enable_if<is_string_type<StringType>::value>::type>
	__inline void write(const StringType& str) {
		write(str.c_str());
	}

	template<class CharType, class = typename std::enable_if<is_char_type<CharType>::value>::type>
	void write(const CharType* output) {
		std::unique_lock<std::mutex> guard(lockMutex);
		std::wofstream log(filePath.c_str(), std::wofstream::app);
		if (log.is_open()) {
			log << output;
			log.flush();
			log.close();
		}
	}
};

class VersionedSynchronizedLogFile : public SynchronizedLogFile {
private:
	static std::wstring CreateVersionedFilePath(std::wstring currentPath) {
		std::wstring pathUpToFileEnding = currentPath.substr(0, currentPath.find_last_of('.'));
		wchar_t buffer[0x80] = { 0x00 };
		time_t timepoint;
		time(&timepoint);
		tm localTimeTm{};
		localtime_s(&localTimeTm, &timepoint);
		swprintf_s(buffer, L"_%i%02i%02i_%02i%02i%02i", (localTimeTm.tm_year + 1900), (localTimeTm.tm_mon + 1), localTimeTm.tm_mday, localTimeTm.tm_hour, localTimeTm.tm_min, localTimeTm.tm_sec);
		return (pathUpToFileEnding + std::wstring(buffer) + currentPath.substr(currentPath.find_last_of('.')));
	}
public:
	VersionedSynchronizedLogFile(std::wstring filePath) : VersionedSynchronizedLogFile(filePath, true) {

	}
	VersionedSynchronizedLogFile(std::wstring filePath, bool clearFile) : SynchronizedLogFile(CreateVersionedFilePath(filePath), clearFile) {

	}
};

class SynchronizedLogFileFactory {
private:
	static std::unordered_map<std::string, std::shared_ptr<SynchronizedLogFile>> LogFiles;
	static std::string DefaultMainInstanceName;
	SynchronizedLogFileFactory() {}
public:
	virtual ~SynchronizedLogFileFactory() {}

	inline static void OnShutdown() {
		LogFiles.clear();
	}

	__inline static void AddInstance(std::string instanceName, std::shared_ptr<SynchronizedLogFile> file) {
		LogFiles.insert_or_assign(instanceName, file);
	}
	__inline static std::shared_ptr<SynchronizedLogFile> GetInstance(std::string instanceName) {
		return (LogFiles.find(instanceName) != LogFiles.cend()) ? LogFiles[instanceName] : nullptr;
	}
	__inline static std::shared_ptr<SynchronizedLogFile> GetDefaultInstance() {
		return GetInstance(DefaultMainInstanceName);
	}
	__inline static void SetDefaultInstance(std::string instanceName) {
		DefaultMainInstanceName = instanceName;
	}
};

class ROSELogger {
protected:
	class ROSELoggerHexValue {
	private:
		uint64_t value;
	public:
		ROSELoggerHexValue(uint64_t value) {
			this->value = value;
		}
		virtual ~ROSELoggerHexValue() {

		}
		__inline uint64_t getValue() const {
			return value;
		}
	};
	class ROSETimeValue {
	private:
		std::tm localTime;
		bool includeSeconds;
	public:
		ROSETimeValue(std::tm value, bool includeSeconds) {
			this->localTime = value;
			this->includeSeconds = includeSeconds;
		}
		virtual ~ROSETimeValue() {

		}
		__inline std::tm getTime() const {
			return localTime;
		}
		inline int getHour() const {
			return localTime.tm_hour;
		}
		inline int getMinutes() const {
			return localTime.tm_min;
		}
		inline int getSeconds() const {
			return localTime.tm_sec;
		}
		inline bool isIncludeSeconds() const {
			return includeSeconds;
		}
	};
private:
	const LogLevel* level;
	std::shared_ptr<SynchronizedLogFile> logFile;
	std::string loggerName;
#ifdef USE_SOURCE_LOCATION
	std::source_location creationLocation;
	std::string loggerFileName;
	std::string tabs;
#endif

	template<typename Arg>
	void logNext(std::wstringstream& out, Arg arg) const {
		out << arg;
	}

	template<typename Arg>
	void logNext(std::wstringstream& out, std::shared_ptr<Arg> sharedPtr) const {
		out << "0x" << std::hex << sharedPtr.get() << std::dec;
	}

	template<>
	void logNext(std::wstringstream& out, std::string str) const {
		out << str.c_str();
	}

	template<>
	void logNext(std::wstringstream& out, ROSETimeValue localTime) const {
		out << localTime.getHour() << ":" << std::setfill((wchar_t)'0') << std::setw(2) << localTime.getMinutes();
		if (localTime.isIncludeSeconds()) {
			out << ":" << localTime.getSeconds();
		}
		out << std::setfill((wchar_t)' ') << std::setw(0);
	}

	template<>
	void logNext(std::wstringstream& out, ROSELoggerHexValue value) const {
		out << std::hex << value.getValue() << std::dec;
	}

	template<>
	void logNext(std::wstringstream& out, std::wstring str) const {
		out << str.c_str();
	}

	template<typename NextType, typename... Arg>
	void logNext(std::wstringstream& out, NextType current, Arg... arg) const {
		logNext(out, current);
		logNext(out, arg...);
	}

	std::string getTimeAsString() const {
		time_t currentTime;
		time(&currentTime);
		tm localTime;
		localtime_s(&localTime, &currentTime);
		std::stringstream stream;
		stream << std::put_time(&localTime, "%H:%M:%S");

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) % 1000;
		stream << ',' << std::setfill('0') << std::setw(3) << ms.count();

		return stream.str();
	}

	std::string getDateTimeAsString() const {
		time_t currentTime;
		time(&currentTime);
		tm localTime;
		localtime_s(&localTime, &currentTime);
		std::stringstream stream;
		stream << std::put_time(&localTime, "%d.%m.%Y|%H:%M:%S");

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) % 1000;
		stream << ',' << std::setfill('0') << std::setw(3) << ms.count();

		return stream.str();
	}

	template<typename... Args>
	void log(const LogLevel* level, Args... args) const {
		if (*level < *(getLogLevel())) {
			return;
		}
		std::wstringstream outputCatcher;
		outputCatcher << getTimeAsString().c_str() << " ";
		colorizeLogLevelOutput(outputCatcher, level);
		outputCatcher << "\t";
#ifdef USE_SOURCE_LOCATION
		outputCatcher << getLoggerCreationFileName().c_str() << getMessageIndentation().c_str();
#else
		if (!loggerName.empty()) {
			outputCatcher << "[" << loggerName.c_str() << "]";
		}
#endif
		outputCatcher << " ";
		logNext(outputCatcher, args...);
		outputCatcher << "\n";
		std::wcout << outputCatcher.str();
		printToLog(outputCatcher);
	}
protected:
	static void colorizeLogLevelOutput(std::wstringstream& output, const LogLevel* level);
	static void colorizeLogLevelOutput(const LogLevel* level);
	void clearLogfile() {
		if (logFile) {
			logFile->clear();
		}
	}
	inline void printToLog(const std::wstringstream& outputCatcher) const {
		printToLog(getLogFileOutput(), outputCatcher);
	}
	static void printToLog(std::shared_ptr<SynchronizedLogFile> logFile, const std::wstringstream& outputCatcher) {
		if (logFile) {
			logFile->write(outputCatcher);
		}
	}
#ifdef USE_SOURCE_LOCATION
	const std::source_location& getLoggerCreationLocation() const {
		return creationLocation;
	}
	const std::string& getLoggerCreationFileName() const {
		return this->loggerFileName;
	}
	const std::string getMessageIndentation() const {
		return tabs;
	}
#endif
public:
#ifdef USE_SOURCE_LOCATION
	ROSELogger(const std::source_location creationLocation = std::source_location::current()) : ROSELogger(creationLocation, LogLevel::DEFAULT_LOGLEVEL) {}
	ROSELogger(const std::source_location location, const LogLevel* newLevel) : creationLocation(location), level(newLevel) {
		loggerFileName = std::string(creationLocation.file_name());
		loggerFileName = loggerFileName.substr(loggerFileName.find_last_of("\\") + 1);
		loggerFileName = loggerFileName.substr(0, loggerFileName.find(".")); 
		loggerFileName = std::string("[") + loggerFileName + std::string("]");
		size_t tabsRequired = (size_t)(std::max)(1, int32_t(5 - (loggerFileName.length() / 8)));
		for (size_t i = 0; i < tabsRequired; i++) {
			tabs.push_back('\t');
		}
#else
	ROSELogger() : ROSELogger(LogLevel::DEFAULT_LOGLEVEL) {}
	ROSELogger(const LogLevel * newLevel) : level(newLevel) {
#endif
		logFile = SynchronizedLogFileFactory::GetDefaultInstance();
	}
	virtual ~ROSELogger() {
		logFile = nullptr;
	}  

	template<typename... Args>
	__inline void logTrace(Args... args) const {
		log(LogLevel::TRACE, args...);
	}

	template<typename... Args>
	__inline void logDebug(Args... args) const {
		log(LogLevel::DEBUG, args...);
	}

	template<typename... Args>
	__inline void logInfo(Args... args) const {
		log(LogLevel::INFO, args...);
	}	
	
	template<typename... Args>
	__inline void logWarn(Args... args) const {
		log(LogLevel::WARNING, args...);
	}	
		
	template<typename... Args>
	__inline void logError(Args... args) const {
		log(LogLevel::ERROR, args...);
	}

	inline static ROSELoggerHexValue asHex(uint32_t value) {
		return ROSELoggerHexValue(value);
	}
	inline static ROSETimeValue asTimeWithSeconds(std::tm timeValue) {
		return ROSETimeValue(timeValue, true);
	}
	inline static ROSETimeValue asTime(std::tm timeValue) {
		return ROSETimeValue(timeValue, false);
	}
	static ROSETimeValue asTime(uint64_t timepointInMilliseconds) {
		time_t timepointInSeconds = (time_t)(timepointInMilliseconds / 1000);
		struct tm t {};
		localtime_s(&t, &timepointInSeconds);
		return asTime(t);
	}
	inline static ROSETimeValue asCurrentTime() {
		return asTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}

	__inline const LogLevel* getLogLevel() const {
		return level;
	}
	__inline void setLogLevel(const LogLevel* newLevel) {
		level = newLevel;
	}
	__inline std::string getLoggerName() const {
		return loggerName;
	}
	__inline void setLoggerName(const char* name) {
		this->loggerName = (name == nullptr ? std::string() : std::string(name));
	}
	__inline std::shared_ptr<SynchronizedLogFile> getLogFileOutput() const {
		return logFile;
	}
	__inline void setLogFileOutput(std::shared_ptr<SynchronizedLogFile> output) {
		this->logFile = output;
	}
};

class ROSEThreadedLogger : public ROSELogger {
private:
	struct LogMessage {
		std::wstringstream output;
		const LogLevel* level;
		std::string loggerName;
		std::shared_ptr<SynchronizedLogFile> logFile;
	};

	static std::mutex ShutdownMutex;
	static std::mutex inputMutex;
	static std::thread LoggerThread;
	static std::deque<LogMessage> streamHolder;
	static std::atomic<int32_t> ReferenceAmount;

	template<typename Arg>
	void logNext(std::wstringstream& wout, const Arg& arg) const {
		wout << arg;
	}

	template<>
	void logNext(std::wstringstream& out, const ROSETimeValue& localTime) const {
		out << localTime.getHour() << ":" << std::setfill((wchar_t)'0') << std::setw(2) << localTime.getMinutes();
		if (localTime.isIncludeSeconds()) {			
			out << ":" << std::setfill((wchar_t)'0') << std::setw(2) << localTime.getSeconds();
		}
		out << std::setfill((wchar_t)' ') << std::setw(0);
	}

	template<>
	void logNext(std::wstringstream& wout, const std::nullptr_t& n) const {
		wout << "nullptr";
	}

	template<>
	void logNext(std::wstringstream& wout, const bool& value) const {
		wout << std::boolalpha << value << std::noboolalpha;
	}

	template<>
	void logNext(std::wstringstream& wout, const std::string& str) const {
		wout << str.c_str();
	}
	template<>
	void logNext(std::wstringstream& wout, const std::wstring& str) const {
		wout << str.c_str();
	}
	template<>
	void logNext(std::wstringstream& wout, const ROSELogger::ROSELoggerHexValue& value) const {
		wout << std::hex << value.getValue() << std::dec;
	}
	template<>
	void logNext(std::wstringstream& wout, const std::_Timeobj<char, const std::tm*>& value) const {
		std::stringstream result;
		result << value;
		wout << result.str().c_str();
	}

	template<typename NextType, typename... Arg>
	void logNext(std::wstringstream& wout, const NextType& current, const Arg&... arg) const {
		logNext(wout, current);
		logNext(wout, arg...);
	}

	template<typename... Args>
	void log(const LogLevel* level, const Args&... args) const {
		if (*level < *(getLogLevel())) {
			return;
		}
		std::wstringstream output;
		logNext(output, args...);
		output << "\n";
		std::wstringstream logOutput;
#ifdef USE_SOURCE_LOCATION
		logOutput << getLoggerCreationFileName().c_str();
		logOutput << getMessageIndentation().c_str();
#else
		if (!getLoggerName().empty()) {
			logOutput << "[" << getLoggerName().c_str() << "]";
		}
#endif
		logOutput << output.str();
		std::unique_lock<std::mutex> lock(inputMutex);
		LogMessage message{ std::move(logOutput), level, getLoggerName(), getLogFileOutput()};
		streamHolder.push_back(std::move(message));
		lock.unlock();
	}

	static std::wstring GetCurrentTimeAsString() {
		auto timeInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		time_t currentTime = time_t(std::chrono::duration_cast<std::chrono::seconds>(timeInMilliseconds).count());

		time(&currentTime);
		tm localTime;
		localtime_s(&localTime, &currentTime);
		std::wstringstream stream;
		auto putTimeResult = std::put_time(&localTime, L"%H:%M:%S");
		stream << putTimeResult;
	
		auto milliseconds = timeInMilliseconds % 1000;
		stream << L'.' << std::setw(3) << std::setfill(L'0') << milliseconds.count() << " ";
		return std::wstring(stream.str());
	}

	
public:
#ifdef USE_SOURCE_LOCATION
	ROSEThreadedLogger(const std::source_location creationLocation = std::source_location::current()) : ROSEThreadedLogger(creationLocation, LogLevel::DEFAULT_LOGLEVEL) {}
	ROSEThreadedLogger(const std::source_location creationLocation, const LogLevel* newLevel) : ROSELogger(creationLocation, newLevel) {
#else
	ROSEThreadedLogger() : ROSEThreadedLogger(LogLevel::DEFAULT_LOGLEVEL) {}
	ROSEThreadedLogger(const LogLevel* newLevel) : ROSELogger(newLevel) {
#endif
	}
	virtual ~ROSEThreadedLogger() {

	}
	static void Init() {
		if (LoggerThread.native_handle()) {
			return;
		}
		ReferenceAmount++;
		LoggerThread = std::thread([&]() {
			ROSEThreadedLogger logger;
			streamHolder = std::deque<LogMessage>();
			while (ReferenceAmount.load() > 0) {
				if (streamHolder.empty()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				else {
					std::deque<LogMessage> copy;
					{
						std::unique_lock<std::mutex> lock(inputMutex);
						copy = std::move(streamHolder);
						streamHolder = std::deque<LogMessage>();
					}
					std::wstringstream output;
					std::shared_ptr<SynchronizedLogFile> lastLogOutput;
					uint64_t lastTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
					std::wstring timepoint = GetCurrentTimeAsString();
					while (!copy.empty()) {
						{
							uint64_t possibleNewTimestamp = std::chrono::system_clock::now().time_since_epoch().count();
							if (lastTimestamp != possibleNewTimestamp) {
								timepoint = GetCurrentTimeAsString();
							}
							const LogMessage& messageEntry = copy.front();
							if (lastLogOutput != messageEntry.logFile) {
								printToLog(lastLogOutput, output);
								lastLogOutput = messageEntry.logFile;
								output.clear();
							}
							output << timepoint.c_str() << " ";
#ifdef _CONSOLE
							std::wcout << timepoint.c_str();
							colorizeLogLevelOutput(messageEntry.level);
							std::wcout << "\t" << messageEntry.output.str().c_str();
#else 
							output << messageEntry.level->getName() << "\t";
#endif
							output << messageEntry.output.str();
						}
						copy.pop_front();
					}
					printToLog(lastLogOutput, output);
				}
			}
		});
	}
	static void Destroy() {
		ReferenceAmount--;
		while (LoggerThread.native_handle() && !LoggerThread.joinable()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		LoggerThread.join();

		delete LogLevel::TRACE;
		delete LogLevel::DEBUG;
		delete LogLevel::INFO;
		delete LogLevel::WARNING;
		delete LogLevel::ERROR;
	}

	static ROSELoggerHexValue asHex(uint32_t value) {
		return ROSELoggerHexValue(value);
	}

	template<typename... Args>
	__inline void logTrace(const Args... args) const {
		log(LogLevel::TRACE, args...);
	}

	template<typename... Args>
	__inline void logDebug(const Args... args) const {
		log(LogLevel::DEBUG, args...);
	}

	template<typename... Args>
	__inline void logInfo(const Args... args) const {
		log(LogLevel::INFO, args...);
	}

	template<typename... Args>
	__inline void logWarn(const Args... args) const {
		log(LogLevel::WARNING, args...);
	}

	template<typename... Args>
	__inline void logError(const Args... args) const {
		log(LogLevel::ERROR, args...);
	}
};

class PerformanceTraceLogger {
private:
	ROSEThreadedLogger logger;
	static std::atomic<bool> TraceEnabled;
public:
	PerformanceTraceLogger() {
		logger.setLogFileOutput(SynchronizedLogFileFactory::GetInstance("PERFORMANCE"));
	}
	virtual ~PerformanceTraceLogger() {}

	template<typename... Args>
	inline void log(const Args... args) const {
		if (TraceEnabled) {
			logger.logInfo(args...);
		}
	}
	static void SetEnabled(bool flag) {
		TraceEnabled = flag;
	}

};

#ifdef _OTHER_ERROR
#define ERROR _OTHER_ERROR
#undef _OTHER_ERROR
#endif

#ifdef _OTHER_DEBUG
#define DEBUG _OTHER_DEBUG
#undef _OTHER_DEBUG
#endif

#endif //__ROSE_LOGGER__