#include "ETagDatabase.h"
#include "../ProjectFilePathHandler.h"

const std::wstring ETagDatabase::DEFAULT_FILE_NAME = std::wstring(L"Icons/etag_db.conf");
const std::wstring ETagDatabase::DEFAULT_FILE_PATH = ProjectFilePathHandler::GetDefaultFilePathUnicode() + ETagDatabase::DEFAULT_FILE_NAME;

ETagDatabase::ETagDatabase(const wchar_t* filePath) : Settings(filePath) {

}

ETagDatabase::~ETagDatabase() {

}