#include "Utility.h"
#include <algorithm>
#include <iterator>
#include <Windows.h>

std::string OverlayUtility::StringUtil::ToAscii(const std::wstring& str) {
	std::string result;
	std::transform(str.begin(), str.end(), std::back_inserter(result), [](wchar_t c) { return (char)c; });
	return result;
}

std::wstring OverlayUtility::StringUtil::ToWideString(const std::string& str) {
	int len = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, str.c_str(), (int)str.length(), NULL, 0);
	std::wstring result = std::wstring(len, 0);
	int resultLength = MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, str.c_str(), (int)str.length(), &result[0], len);
	return result;
}

std::string OverlayUtility::StringUtil::toHtmlString(const std::wstring& str) {
	int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0, NULL, 0);
	std::string result = std::string(len*2+1, 0);
	int resultLength = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), &result[0], len, 0, 0);
	return result;
}

std::string OverlayUtility::StringUtil::StringViewToString(const std::string_view& view) {
	return std::string(view.begin(), view.end());
}


std::string OverlayUtility::StringUtil::RemoveCharactersFromString(const std::string& str, std::vector<char> charactersToCheckFor) {
	return RemoveCharactersFromString(str.c_str(), charactersToCheckFor);
}

std::string OverlayUtility::StringUtil::ToLower(const std::string& convertToLowerStr) {
	std::string result = convertToLowerStr;
	std::transform(result.begin(), result.end(), result.begin(), [](const char c) { return tolower(c); });
	return result;
}

std::string OverlayUtility::StringUtil::RemoveCharactersFromString(const char* str, std::vector<char> charactersToCheckFor) {
	std::string output;
	output.reserve(strlen(str));
	while(char currentCharacter = *str++) {
		bool characterToRemove = false;
		for (auto c : charactersToCheckFor) {
			if (currentCharacter == c) {
				characterToRemove = true;
				break;
			}
		}
		if (!characterToRemove) {
			output += currentCharacter;
		}
	}
	return output;
}