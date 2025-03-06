#include "PatchVersionChecker.h"
#include "../Common/Data/CURLWrapper.h"
#include "../Common/Logging/Logger.h"
#include "../Common/IO/FileReader.h"
#include "../Common/ProjectFilePathHandler.h"
#include "../Common/ProcessWatcher.h"
#include <windows.h>

void writeNewPatchVersionLocally(const std::string& newVersion) {
	std::string fileName = ProjectFilePathHandler::GetDefaultFilePath() + std::string("patch.version");
	FILE* fh;
	fopen_s(&fh, fileName.c_str(), "wb+");
	if (!fh) {
		return;
	}
	fputs(newVersion.c_str(), fh);
	fclose(fh);
}

void removePatchArchive(const std::wstring& patchArchivePath) {
	std::wstring removeCommand = std::wstring(L"powershell Remove-Item -Path ") + patchArchivePath;
	_wsystem(removeCommand.c_str());
}

void applyNewPatch(const std::wstring& filePath, const std::string& newVersion) {
	ROSEThreadedLogger logger;
	std::wstring commandLine = L"powershell Expand-Archive -Force ";
	commandLine += filePath;
	commandLine += L" -DestinationPath " + ProjectFilePathHandler::GetDefaultFilePathUnicode();
	logger.logInfo("Applying path on path: ", ProjectFilePathHandler::GetDefaultFilePathUnicode().c_str());
	int result = _wsystem(commandLine.c_str());
	if (result >= 0) {
		writeNewPatchVersionLocally(newVersion);
		logger.logInfo("Successfully applied patch locally. Removing patch archive...");
		removePatchArchive(filePath.c_str());
	}
}


void backupCurrentState(const std::string& oldVersion) {
	ROSEThreadedLogger logger;
	logger.logInfo("Creating backup of current version (", oldVersion.c_str(), ")");
	logger.logDebug("Creating patch backup path: ", ProjectFilePathHandler::Patches::GetDefaultFilePathUnicode().c_str());
	_wmkdir(ProjectFilePathHandler::Patches::GetDefaultFilePathUnicode().c_str());

	std::wstring oldVersionEncoded = std::wstring(oldVersion.begin(), oldVersion.end());
	std::wstring baseCommandLine = L"powershell Compress-Archive -Path ";
	std::wstring basePath = ProjectFilePathHandler::GetDefaultFilePathUnicode();
	std::wstring patchBasePath = ProjectFilePathHandler::Patches::GetDefaultFilePathUnicode();
	std::wstring designPath = ProjectFilePathHandler::Design::GetDefaultFilePathUnicode();
	std::wstring fontPath = ProjectFilePathHandler::Fonts::GetDefaultFilePathUnicode();
	std::wstring packaging = baseCommandLine + designPath + std::wstring(L", ") + fontPath + std::wstring(L", ") + (basePath  + std::wstring(L"*.exe, ")) + (basePath + std::wstring(L"*.version ")) + std::wstring(L" -Force -DestinationPath ") + patchBasePath + std::wstring(L"LoLOverlay_") + oldVersionEncoded + std::wstring(L".zip");
	_wsystem(packaging.c_str());
	logger.logInfo("Creating backup finished.");
}

void downloadNewPatch(PatchVersionChecker* checker) {
	ROSEThreadedLogger logger;
	backupCurrentState(checker->getLocalVersion());
	std::wstring filePath = ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"patch.zip");
	CURLFileOutputWrapper wrapper(checker->getPatchUrl().c_str(), filePath.c_str());
	wrapper.setSslEnabled(true);
	wrapper.setSaveHeaderResponse(true);
	logger.logInfo("Locally used patch (", checker->getLocalVersion().c_str(), ") is outdated. Applying newest patch: ", checker->getExternalVersion().c_str());
	logger.logDebug("Getting newest patch from URL: ", checker->getPatchUrl().c_str());
	if (wrapper.fireRequest()) {
		logger.logInfo("Downloading patch finished at ", wrapper.getDownloadSpeed(), "kb/s to path: ", filePath);
		applyNewPatch(filePath, checker->getExternalVersion());
	}		
}

int main() {
	auto overlayProcess = ProcessWatcher::GetProcess(L"BroadcasterMain.exe");
	if (overlayProcess.isValid()) {
		logger.logInfo("Waiting for BroadcasterMain.exe to stop running.");
	}
	while(overlayProcess.isValid()) {
		overlayProcess = ProcessWatcher::GetProcessByProcessId(overlayProcess.getProcessId());
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	logger.logInfo("Checking for newer patches...");
	PatchVersionChecker checker;
	if (checker.isVersionOutdated()) {
		downloadNewPatch(&checker);
		logger.logInfo("Restarting overlay process.");
		ProcessWatcher::StartProcessFrom(ProjectFilePathHandler::GetDefaultFilePathUnicode().c_str(), L"BroadcasterMain.exe");
	}
	else {
		logger.logInfo("Current version (", checker.getLocalVersion().c_str(), ") is up to date.");
	}
	logger.logInfo("Press any key to finish patcher execution.");
	system("pause");
	ROSEThreadedLogger::Destroy();
	return 0;
}