#include "ProjectFilePathHandler.h"
#include <algorithm>
#include <direct.h>

std::string ProjectFilePathHandler::GetDefaultFilePath() {
#ifdef _DEBUG
	std::string thisFilePath = __FILE__;
	std::string commonFolder = thisFilePath.substr(0, thisFilePath.find_last_of('\\'));
	std::string sourceMainFolder = commonFolder.substr(0, commonFolder.find_last_of('\\'));
	std::string aboveSourceFolder = sourceMainFolder.substr(0, sourceMainFolder.find_last_of('\\') + 1);
	return (aboveSourceFolder + std::string("WhalepowerBroadcaster\\RELEASE\\"));
#else
	return std::string("./");
#endif
}

std::wstring ProjectFilePathHandler::GetDefaultFilePathUnicode() {
	std::string asciiFilePath = GetDefaultFilePath();
	return std::wstring(asciiFilePath.c_str(), asciiFilePath.c_str() + asciiFilePath.length());
}