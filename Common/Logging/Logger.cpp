#include "Logger.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef ERROR
#undef ERROR
#endif

const LogLevel* LogLevel::TRACE = new LogLevel("TRACE", 0, LogLevelTextColorSchema::GREEN);
const LogLevel* LogLevel::DEBUG = new LogLevel("DEBUG", 1, LogLevelTextColorSchema::BLUE);
const LogLevel* LogLevel::INFO = new LogLevel("INFO ", 2, LogLevelTextColorSchema::WHITE);
const LogLevel* LogLevel::WARNING = new LogLevel("WARN ", 3, LogLevelTextColorSchema::ORANGE);
const LogLevel* LogLevel::ERROR = new LogLevel("-ERROR-", 4, LogLevelTextColorSchema::RED);


std::atomic<bool> PerformanceTraceLogger::TraceEnabled = false;

std::unordered_map<std::string, std::shared_ptr<SynchronizedLogFile>> SynchronizedLogFileFactory::LogFiles;
std::string SynchronizedLogFileFactory::DefaultMainInstanceName = std::string();

#ifdef _DEBUG
const LogLevel* LogLevel::DEFAULT_LOGLEVEL = LogLevel::DEBUG;
#else
const LogLevel* LogLevel::DEFAULT_LOGLEVEL = LogLevel::DEBUG;
#endif

void ROSELogger::colorizeLogLevelOutput(std::wstringstream& output, const LogLevel* level) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, level->getColorScheme());
	output << level->getName();
	SetConsoleTextAttribute(hConsole, static_cast<uint8_t>(LogLevelTextColorSchema::DEFAULT));
}

void ROSELogger::colorizeLogLevelOutput(const LogLevel* level) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, level->getColorScheme());
	std::wcout << level->getName();
	SetConsoleTextAttribute(hConsole, static_cast<uint8_t>(LogLevelTextColorSchema::DEFAULT));
}

SynchronizedLogFile::SynchronizedLogFile(std::wstring filePath, bool clearFile) {
	this->filePath = filePath;
	if (clearFile) {
		clear();
	}
}

const char *colors[] = {
	"\x1b[0m",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"\x1b[37m", //WHITE
	nullptr,
	nullptr,
	"\x1b[32m", //GREEN
	"\x1b[34m", //BLUE
	"\x1b[31m", //RED
	nullptr,
	"\x1b[33m",
	"\x1b[1m"
};

std::mutex ROSEThreadedLogger::inputMutex;
std::thread ROSEThreadedLogger::LoggerThread; 
std::atomic<int32_t> ROSEThreadedLogger::ReferenceAmount{ 0 };
std::deque<ROSEThreadedLogger::LogMessage> ROSEThreadedLogger::streamHolder;