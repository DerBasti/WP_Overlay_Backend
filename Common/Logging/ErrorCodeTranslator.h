#ifndef __LEAGUE_ERROR_CODE_TRANSLATOR__
#define __LEAGUE_ERROR_CODE_TRANSLATOR__

#include <Windows.h>
#include <string>

class ErrorCodeTranslator {
private:
	ErrorCodeTranslator() {}
public:
	static std::string GetErrorCodeString() {
		return GetErrorCodeString(GetLastError());
	}
	static std::string GetErrorCodeString(DWORD errorCode) {
		LPSTR messageBuffer = nullptr;
		//Ask Win32 to give us the string version of that message ID.
		//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		//Copy the error message into a std::string.
		std::string message(messageBuffer, size);

		//Free the Win32's string's buffer.
		LocalFree(messageBuffer);
		while (!message.empty() && (message.back() == ' ' || message.back() == '\n' || message.back() == '\r')) {
			message.pop_back();
		}
		return message;
	}
};

#endif 