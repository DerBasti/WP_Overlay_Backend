#ifndef __OVERLAY_UTILITY__
#define __OVERLAY_UTILITY__

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <future>

namespace OverlayUtility {
	class StringUtil {
		private: 
			StringUtil() {}
		public:
			static std::string toHtmlString(const std::wstring& str);
			static std::wstring ToWideString(const std::string& str);
			static std::string ToAscii(const std::wstring& str);
			static std::string StringViewToString(const std::string_view& view);
			static std::string ToLower(const char* convertToLowerStr) {
				return ToLower(std::string(convertToLowerStr));
			}
			static std::string ToLower(const std::string& convertToLowerStr);
			static std::string RemoveCharactersFromString(const std::string& str, std::vector<char> charactersToCheckFor);
			static std::string RemoveCharactersFromString(const char* str, std::vector<char> charactersToCheckFor);
	};
	class FutureUtil {
		private:
			FutureUtil() {}
		public:
			template<class _T>
			static bool IsFutureResultReady(const std::future<_T>& future) { return future.valid() && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready; }
#ifndef AwaitTillFutureReady
	#define AwaitTillFutureReady(future) while (!OverlayUtility::FutureUtil::IsFutureResultReady(future)) {	std::this_thread::sleep_for(std::chrono::milliseconds(10));	}
#endif
	};
}

#endif //__OVERLAY_UTILITY__