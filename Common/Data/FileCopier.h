#ifndef __LEAGUE_FILE_COPIER__
#define __LEAGUE_FILE_COPIER__

#include <fstream>
#include <string>
#include <Windows.h>
#include "../Logging/Logger.h"

class FileCopier {
private:
	FileCopier();
	static std::wstring ExtractTargetDirectory(const wchar_t* target) {
		std::wstring targetAsString = std::wstring(target);
		size_t position = targetAsString.find_last_of('\\');
		if (position == std::wstring::npos) {
			position = targetAsString.find_last_of('/');
		}
		std::wstring targetDirectory = targetAsString.substr(0, position+1);
		return targetDirectory;
	}
public:
	static bool Copy(const wchar_t* fromFile, const wchar_t* toFile) {
		if (_wcsicmp(fromFile, toFile) == 0) {
			return true;
		}
		ROSEThreadedLogger logger;
		std::wstring targetDirectory = ExtractTargetDirectory(toFile);
		if (CreateDirectory(targetDirectory.c_str(), nullptr) != 0) {
			logger.logDebug("Added target directory: ", targetDirectory.c_str());
		}

		std::ifstream src(fromFile, std::ios::binary);
		std::ofstream target(toFile, std::ios::binary);

		target << src.rdbuf();
		return src.tellg() == target.tellp();
	}
};

#endif //__LEAGUE_FILE_COPIER__