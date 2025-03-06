#include "RiotLockFile.h"
#include "../../Common/IO/FileReader.h"

RiotLockFile::RiotLockFile() {
	invalidFlag = true;
}

RiotLockFile::RiotLockFile(const std::string& lolPath) : RiotLockFile() {
	std::string realPath = lolPath;
	if (!realPath.empty() && realPath.back() != '\\') {
		realPath += '\\';
	}
	FileInputReader reader((realPath + std::string("lockfile")).c_str());
	readLockFile(reader);
}

RiotLockFile::RiotLockFile(const std::wstring& lolPath) : RiotLockFile() {
	std::wstring realPath = lolPath;
	if (!realPath.empty() && realPath.back() != '\\') {
		realPath += '\\';
	}
	UnicodeFileInputReader reader((realPath + std::wstring(L"lockfile")).c_str());
	readLockFile(reader);
}

RiotLockFile::~RiotLockFile() {

}

void RiotLockFile::readLockFile(DataReader& reader) {
	if (reader.isValid()) {
		auto string = reader.readStringWrapped(reader.getFileSize());
		readValuesFromLockFile(string);
		invalidFlag = lockFileValues.size() != LockFileTokenId::EXPECTED_AMOUNT;
	}
}

void RiotLockFile::readValuesFromLockFile(std::shared_ptr<char>& lockFileContentShared) {
	std::string lockFileStringContent = std::string(lockFileContentShared.get());
	size_t nextTokenPosition = 0;
	do {
		nextTokenPosition = lockFileStringContent.find(':');
		std::string nextContent = lockFileStringContent.substr(0, nextTokenPosition);
		//logger.logDebug("[RiotLockFile] Parsed value: '", nextContent.c_str(), "'");
		lockFileStringContent = lockFileStringContent.substr(nextTokenPosition + 1);
		lockFileValues.push_back(std::move(nextContent));
	} while (nextTokenPosition != std::string::npos);
}