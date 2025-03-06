#ifndef __CRASH_HANDLER__
#define __CRASH_HANDLER__
#pragma once

#include <string>
#include <functional>
#if _MSVC_LANG >= 202002
	#ifndef USE_SOURCE_LOCATION
		#define USE_SOURCE_LOCATION
	#endif
	#include <source_location>
#endif

class CrashHandler {
private:
	static std::wstring CrashHandlerName;

	static void handleCrashSignal(const std::string& message);
	static void handleNullpointer(int signal);
	static void handleIllegalOperation(int signal);
	
	CrashHandler();
public:
	virtual ~CrashHandler();
	static void AttachSignalHandler();
	static void AttachSignalHandler(std::wstring crashHandlerName);
	static void HandleTermination();
#ifdef USE_SOURCE_LOCATION
	static void HandleTermination(const std::source_location& location);
#endif
};

#endif //__CRASH_HANDLER__