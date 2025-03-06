#ifndef __ROSE_DIRECTORY_PARSER__
#define __ROSE_DIRECTORY_PARSER__
#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <chrono>
#include <algorithm> 

class Directory {
private:
	std::string path;
	std::vector<std::shared_ptr<Directory>> subDirectories;
	std::vector<std::string> files;
public:
	Directory(const std::string& path) {
		this->path = path;
	}
	virtual ~Directory() {

	}
};

class DeletedFile {
private:
	std::string filePath;
	std::chrono::milliseconds timePassedSinceLastWrite;
public:
	constexpr static std::chrono::hours EXPIRATION_HOURS = std::chrono::hours(336);
	DeletedFile(std::string filePath, std::chrono::milliseconds timePassedSinceLastWriteInMilliseconds) {
		this->filePath = filePath;
		this->timePassedSinceLastWrite = timePassedSinceLastWriteInMilliseconds;
	}
	virtual ~DeletedFile() {

	}
	__inline std::string getFilePath() const {
		return filePath;
	}
	__inline std::chrono::milliseconds getTimeOverThreshold() const {
		return timePassedSinceLastWrite;
	}
	__inline float getTimeOverThresholdInHours() const {
		return std::chrono::duration_cast<std::chrono::duration<float, std::ratio<3600i64>>>(timePassedSinceLastWrite).count();
	}
};

class DirectoryParser {
private:
	std::string searchPath;
	std::string fileExtension;
	std::vector<std::string> listOfFiles;
	std::vector<std::wstring> listOfFilesInUnicode;
	std::vector<std::string> folders;
	std::vector<std::wstring> foldersInUnicode;
	bool searchPathExistent;

	void convertResultToUnicode() {
		for (auto file : listOfFiles) {
			std::wstring unicodeResult = std::wstring(file.c_str(), file.c_str() + file.length());
			listOfFilesInUnicode.push_back(std::move(unicodeResult));
		}
		for (auto directory : folders) {
			std::wstring unicodeResult = std::wstring(directory.c_str(), directory.c_str() + directory.length());
			foldersInUnicode.push_back(std::move(unicodeResult));
		}
	}
protected:
	virtual std::vector<std::string> parseDirectoryRecursively(const std::string& path) {
		return std::vector<std::string>();
	}
	void addDirectoryToResult(const std::string& folderPath) {
		folders.push_back(folderPath);
	}
	virtual std::vector<std::string> parseDirectory(const std::string& path) const {
		return std::vector<std::string>();
	}
	virtual void validateSearchPath() {

	}
	bool isFileExtensionMatching(const std::string& fileName) const {
		int64_t position = fileName.find_last_of('.');
		bool found = false;
		if (position >= 0) {
			const char* fileNamePtr = &(fileName.data()[position]);
			return _stricmp(fileNamePtr, fileExtension.data()) == 0;
		}
		return false;
	}
	__inline bool isFileExtensionSet() const {
		return fileExtension.length()>=2;
	}
	void setPathExistent(bool validFlag) {
		searchPathExistent = validFlag;
	}
public:
	DirectoryParser(const char *searchPath) : DirectoryParser(searchPath, nullptr) {
	}
	DirectoryParser(const char* searchPath, const char* fileExtension) {
		setSearchPath(searchPath);
		setFileExtensionToSearchFor(fileExtension);
	}
	virtual ~DirectoryParser() {

	}
	void loadFileList() {
		std::function<std::vector<std::string>(const std::string&)> searchMethod;
		if (isRecursiveSearchEnabled()) {
			searchMethod = std::bind(&DirectoryParser::parseDirectoryRecursively, this, std::placeholders::_1);
		}
		else {
			searchMethod = std::bind(&DirectoryParser::parseDirectory, this, std::placeholders::_1);
		}
		listOfFiles = std::move(searchMethod(getSearchPath()));
		convertResultToUnicode();
	}
	__inline static bool isFileExistent(const char* path) {
#ifdef _WIN32
		WIN32_FIND_DATAA ffd = { 0x00 };
		HANDLE handle = FindFirstFileA(path, &ffd);
		bool result = handle != INVALID_HANDLE_VALUE && (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
		FindClose(handle);
		return result;
#else
		return false;
#endif
	}
	virtual std::vector<DeletedFile> deleteFilesBeforeTimepoint(const std::vector<std::string>& fileList, std::chrono::milliseconds timePassed) {
		return std::vector<DeletedFile>();
	}
	__inline bool isRecursiveSearchEnabled() const {
		return true;
	}
	__inline const std::vector<std::string>& getFileList() const {
		return listOfFiles;
	}
	__inline const std::vector<std::wstring>& getFileListAsUnicode() const {
		return listOfFilesInUnicode;
	}
	__inline const std::vector<std::string>& getFolderList() const {
		return folders;
	}
	__inline std::string getSearchPath() const {
		return searchPath;
	}
	__inline void setSearchPath(const char* searchPath) {
		setSearchPath(searchPath == nullptr ? std::string("") : std::string(searchPath));
	}
	__inline void setSearchPath(const std::string& searchPath) {
		this->searchPath = searchPath;
	}
	__inline std::string getFileExtensionToSearchFor() const {
		return fileExtension;
	}
	__inline void setFileExtensionToSearchFor(const char* extension) {
		setFileExtensionToSearchFor(extension == nullptr ? std::string("") : std::string(extension));
	}
	__inline void setFileExtensionToSearchFor(const std::string& extension) {
		std::string extensionToSet = extension;
		if (extension.length() > 0 && extension.at(0) != '.') {
			extensionToSet = std::string(".") + extension;
		}
		this->fileExtension = extensionToSet;
	}
	__inline bool isPathExistent() const {
		return searchPathExistent;
	}
};

class WindowsDirectoryParser : public DirectoryParser {
protected:
	virtual std::vector<std::string> parseDirectoryRecursively(const std::string& path) {
		WIN32_FIND_DATAA ffd = { 0x00 };
		std::vector<std::string> result;
		std::string realPath = path.at(path.length() - 1) == '\\' ? path : (path + std::string("\\"));
		HANDLE handle = FindFirstFileA((realPath + std::string("\\*")).c_str(), &ffd);
		do {
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				std::string subPath = realPath + std::string(ffd.cFileName);
				if (subPath.find_last_of(".") != subPath.length() - 1) {
					addDirectoryToResult(subPath);
					std::vector<std::string> subfiles = parseDirectoryRecursively(subPath);
					result.insert(result.end(), subfiles.begin(), subfiles.end());
				}
			}
			else {
				std::string fileName = std::string(ffd.cFileName);
				if (!fileName.empty() && !isFileExtensionSet() || isFileExtensionMatching(fileName)) {
					result.push_back(realPath + std::string(ffd.cFileName));
				}
			}
		} while (FindNextFileA(handle, &ffd) != 0);
		FindClose(handle);
		return result;
	}
	virtual std::vector<std::string> parseDirectory(const std::string& path) const {
		return std::vector<std::string>();
	}
	virtual void validateSearchPath() {
		WIN32_FIND_DATAA ffd = { 0x00 };
		std::vector<std::string> result;
		HANDLE handle = FindFirstFileA((getSearchPath() + std::string("\\*")).c_str(), &ffd);
		setPathExistent(handle != INVALID_HANDLE_VALUE);
		FindClose(handle);
	}
public:
	WindowsDirectoryParser(const char *searchPath) : WindowsDirectoryParser(searchPath, nullptr) {
	}
	WindowsDirectoryParser(const char* searchPath, const char* fileExtension) : DirectoryParser(searchPath, fileExtension) {
		validateSearchPath();
	}
	virtual ~WindowsDirectoryParser() {

	}
	static std::wstring GetAbsolutePath(const wchar_t* relativePath) {
		wchar_t buffer[0x200] = { 0x00 };
		GetFullPathNameW(relativePath, 0x200, buffer, nullptr);
		return std::wstring(buffer);
	}
	virtual std::vector<DeletedFile> deleteFilesBeforeTimepoint(const std::vector<std::string>& fileList, std::chrono::milliseconds timePassedRequired) {
		std::vector<DeletedFile> deletedFiles;
		for (auto currentFile : fileList) {
			WIN32_FIND_DATAA ffd = { 0x00 };
			HANDLE handle = FindFirstFileA(currentFile.c_str(), &ffd);
			if (!handle) {
				continue;
			}
			bool deleteFileRequired = false;
			if (handle && !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				SYSTEMTIME st;
				FileTimeToSystemTime(&ffd.ftLastWriteTime, &st);
				std::tm t;
				t.tm_sec = st.wSecond;
				t.tm_min = st.wMinute;
				t.tm_hour = st.wHour;
				t.tm_mday = st.wDay;
				t.tm_mon = st.wMonth - 1;
				t.tm_year = st.wYear - 1900;
				t.tm_isdst = 0;
				std::chrono::system_clock::time_point dateTime =
					std::chrono::system_clock::from_time_t(mktime(&t)) + std::chrono::duration<uint64_t, std::milli>(st.wMilliseconds);

				auto timePassedSinceLastWriteInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - dateTime);
				if (timePassedSinceLastWriteInMilliseconds.count() > timePassedRequired.count()) {
					deletedFiles.push_back(std::move(DeletedFile(currentFile, timePassedSinceLastWriteInMilliseconds)));
					deleteFileRequired = true;
				}
			}
			FindClose(handle);
			if (deleteFileRequired) {
				DeleteFileA(currentFile.c_str());
			}
		}
		return deletedFiles;
	}
};

#endif //__ROSE_DIRECTORY_PARSER__