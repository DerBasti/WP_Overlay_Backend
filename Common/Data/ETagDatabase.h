#ifndef __ETAG_DATABASE__
#define __ETAG_DATABASE__
#pragma once

#include "../IO/FunctionSettings.h"

class ETagDatabase : public Settings {
private:
	static const std::wstring DEFAULT_FILE_PATH;
	static const std::wstring DEFAULT_FILE_NAME;
public:
	ETagDatabase() : ETagDatabase(DEFAULT_FILE_PATH.c_str()) {}
	ETagDatabase(const wchar_t* filePath);
	virtual ~ETagDatabase();
};

#endif//__ETAG_DATABASE__