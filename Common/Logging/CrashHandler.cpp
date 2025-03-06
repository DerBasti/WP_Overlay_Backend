#include "CrashHandler.h"
#include "../ProjectFilePathHandler.h"
#include "../Logging/Logger.h"

#include <csignal>
#include <ShlObj.h>
#if _MSVC_LANG < 202004L
#include "../Logging/StackWalker.h"
#include <Psapi.h>
#else
#include <stacktrace>
#endif


std::wstring CrashHandler::CrashHandlerName = std::wstring();

CrashHandler::CrashHandler() {
}

CrashHandler::~CrashHandler() {

}


void CrashHandler::handleCrashSignal(const std::string& message) {
	std::string totalErrorMessage = message + std::string("\n======================\nStackTrace: \n");
#if _MSVC_LANG < 202004L
	StackWalker stack;
	std::string exceptionStackTrace;
	for (auto& it : stack.getEntries()) {
		exceptionStackTrace += (it ? it->toString().c_str() : "???");
		exceptionStackTrace += '\n';
	}
	totalErrorMessage += exceptionStackTrace;
#else
	auto currentStackTrace = std::stacktrace::current();
	for (auto& entry : currentStackTrace) {
		totalErrorMessage += std::to_string(entry) + std::string("\n");
	}
#endif
	std::wstring crashLogName = ProjectFilePathHandler::Logs::Crash::GetDefaultFilePathUnicode() + CrashHandlerName + std::wstring(L"Crash.log");
	VersionedSynchronizedLogFile logFile(crashLogName.c_str());
	logFile.write(totalErrorMessage.c_str());
	/*std::wstring selectCommand = std::wstring(L"explorer.exe /select,\"");
	selectCommand += crashLogName;
	selectCommand += std::wstring(L"\"");
	_wsystem(selectCommand.c_str());
	ShellExecuteW(nullptr, nullptr, L"https://drive.google.com/drive/folders/1FETxXJqwCuJcpKZL3fxC9Z20TkrTLebC", nullptr, nullptr, SW_SHOW);
	*/
}
void CrashHandler::handleNullpointer(int signal) {
	auto exceptionPointer = (_EXCEPTION_POINTERS**)__pxcptinfoptrs();
	handleCrashSignal("Segmentation fault detected.");
}

void CrashHandler::handleIllegalOperation(int signal) {
	handleCrashSignal("Illegal operation detected.");
}

void CrashHandler::AttachSignalHandler(std::wstring crashHandlerName) {
	CrashHandlerName = crashHandlerName;
	AttachSignalHandler();
}

void CrashHandler::AttachSignalHandler() {
	std::signal(SIGINT, [](int signal) { return handleIllegalOperation(signal); });
	std::signal(SIGILL, [](int signal) { return handleIllegalOperation(signal); });
	std::signal(SIGSEGV, [](int signal) { return handleNullpointer(signal); });
	std::signal(SIGTERM, [](int signal) { return handleNullpointer(signal); });
	std::signal(SIGABRT, [](int signal) { return handleNullpointer(signal); });
	std::set_terminate(CrashHandler::HandleTermination);
}

void CrashHandler::HandleTermination() {
	try {
		if (std::current_exception() != nullptr) {
			std::rethrow_exception(std::current_exception());
		}
		handleCrashSignal("Unexpected termination without exception is occuring.");
	}
	catch (std::exception& ex) {
		const char* exMsg = ex.what();
		std::string error = std::string("Uncaught exception was thrown: ") + std::string(exMsg == nullptr ? "" : ex.what());
		handleCrashSignal(error.c_str());
	}
	catch (...) {
		std::string error = std::string("Uncaught exception was thrown.");
		handleCrashSignal(error.c_str());
	}
}

#ifdef USE_SOURCE_LOCATION
void CrashHandler::HandleTermination(const std::source_location& location) {
	char buffer[0x200] = { 0x00 };
	sprintf_s(buffer, "Uncaught exception was thrown (in: %s:%s(%i)): ", location.file_name(), location.function_name(), location.line());
	try {
		if (std::current_exception() != nullptr) {
			std::rethrow_exception(std::current_exception());
		}
		sprintf_s(buffer, "Unexpected termination was called from %s:%s(%i) ", location.file_name(), location.function_name(), location.line());
		handleCrashSignal(buffer);
	}
	catch (std::exception& ex) {
		const char* exMsg = ex.what();
		const char* exceptionName = typeid(*(&ex)).name();
		sprintf_s(buffer, "Uncaught exception was thrown (in: %s:%s(%i)): ", location.file_name(), location.function_name(), location.line());
		std::string error = std::string(buffer) + std::string(exMsg == nullptr ? "EMPTY_EXCEPTION_MESSAGE" : ex.what());
		handleCrashSignal(error.c_str());
	}
	catch (...) {
		sprintf_s(buffer, "Uncaught exception was thrown (in: %s:%s(%i)): ", location.file_name(), location.function_name(), location.line());
		std::string error = std::string(buffer);
		handleCrashSignal(error.c_str());
	}
}
#endif