#ifndef __LEAGUE_DEBUG_FILEPATH_HANDLER__
#define __LEAGUE_DEBUG_FILEPATH_HANDLER__

#include <string>
#include <algorithm>
#include <direct.h>

class ProjectFilePathHandler {
	ProjectFilePathHandler() {}
public:
	~ProjectFilePathHandler() {}
	static std::string GetDefaultFilePath();
	static std::wstring GetDefaultFilePathUnicode();

	class Logs {
		private:
			Logs() {}
		public:
			static std::string GetDefaultFilePath() {
				return ProjectFilePathHandler::GetDefaultFilePath() + std::string("Logs/");
			}

			static std::wstring GetDefaultFilePathUnicode() {
				return ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Logs/");
			}

			class Crash {
			private:
				Crash() {}
			public:
				static std::string GetDefaultFilePath() {
					return ProjectFilePathHandler::Logs::GetDefaultFilePath() + std::string("Crash/");
				}
				static std::wstring GetDefaultFilePathUnicode() {
					return ProjectFilePathHandler::Logs::GetDefaultFilePathUnicode() + std::wstring(L"Crash/");
				}
			};
	};

	class Design {
		private:
			Design() {}
		public:
			static std::string GetDefaultFilePath() {
				return ProjectFilePathHandler::GetDefaultFilePath() + std::string("Design/");
			}

			static std::wstring GetDefaultFilePathUnicode() {
				return ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Design/");
			}
	};

	class Layout {
		private:
			Layout() {}
		public:
			static std::string GetDefaultFilePath() {
				return ProjectFilePathHandler::Design::GetDefaultFilePath() + std::string("Layout/");
			}

			static std::wstring GetDefaultFilePathUnicode() {
				return ProjectFilePathHandler::Design::GetDefaultFilePathUnicode() + std::wstring(L"Layout/");
			}
	};

	class Fonts {
		private:
			Fonts() {}
		public:
			static std::string GetDefaultFilePath() {
				return ProjectFilePathHandler::GetDefaultFilePath() + std::string("Fonts/");
			}

			static std::wstring GetDefaultFilePathUnicode() {
				return ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Fonts/");
			}
	};

	class Patches {
		private:
			Patches() {}
		public:
			static std::string GetDefaultFilePath() {
				return ProjectFilePathHandler::GetDefaultFilePath() + std::string("Patches/");
			}
			static std::wstring GetDefaultFilePathUnicode() {
				return ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Patches/");
			}
	};

	class PrimeleagueTeams {
	private:
		PrimeleagueTeams() {}
	public:
		static std::string GetDefaultFilePath() {
			return ProjectFilePathHandler::GetDefaultFilePath() + std::string("Primeleague/");
		}
		static std::wstring GetDefaultFilePathUnicode() {
			return ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Primeleague/");
		}
	};

	static void CreateFullPath(std::string path) {
		size_t lastKnownPosition = 0;
		while (path.find("/", lastKnownPosition) != std::string::npos || path.find("\\", lastKnownPosition) != std::string::npos) {
			lastKnownPosition = (std::min)(path.find("/", lastKnownPosition), path.find("\\", lastKnownPosition));
			std::string currentFolder = path.substr(0, lastKnownPosition);
			lastKnownPosition += 1;
			_mkdir(currentFolder.c_str());
		}
		_mkdir(path.c_str());
	}

	static void CreateFullPathUnicode(std::wstring path) {
		size_t lastKnownPosition = 0;
		while (path.find(L"/", lastKnownPosition) != std::string::npos || path.find(L"\\", lastKnownPosition) != std::string::npos) {
			lastKnownPosition = (std::min)(path.find(L"/", lastKnownPosition), path.find(L"\\", lastKnownPosition));
			std::wstring currentFolder = path.substr(0, lastKnownPosition);
			lastKnownPosition += 1;
			_wmkdir(currentFolder.c_str());
		}
		_wmkdir(path.c_str());
	}
};

#endif //__LEAGUE_DEBUG_FILEPATH_HANDLER__