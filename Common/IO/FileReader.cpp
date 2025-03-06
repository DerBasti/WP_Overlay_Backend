#include "FileReader.h"
#ifdef _WIN32
#include <Windows.h>
#endif

FileReader::FileReader(const char *path) {
	int filePathLength = path == nullptr ? 0 : static_cast<int>(strlen(path));
	char *pathHolder = new char[filePathLength + 1];
	strncpy_s(pathHolder, filePathLength + 1, (path == nullptr ? "" : path), filePathLength);
	pathHolder[filePathLength] = 0x00;
	this->filePath = std::shared_ptr<char>(std::move(pathHolder), std::default_delete<char[]>());
#ifdef _WIN32
	char* absolutePathHolder = new char[0x200];
	GetFullPathNameA(path, 0x200, absolutePathHolder, nullptr);
	this->absoluteFilePath = std::shared_ptr<char>(std::move(absolutePathHolder), std::default_delete<char[]>());
#endif
}

FileReader::~FileReader() {

}


UnicodeFileReader::UnicodeFileReader(const wchar_t *path) {
	int filePathLength = path == nullptr ? 0 : static_cast<int>(wcslen(path));
	wchar_t *pathHolder = new wchar_t[filePathLength + 1];
	wcsncpy_s(pathHolder, filePathLength + 1, (path == nullptr ? L"" : path), filePathLength);
	pathHolder[filePathLength] = 0x00;
	this->filePath = std::shared_ptr<wchar_t>(std::move(pathHolder), std::default_delete<wchar_t[]>());
#ifdef _WIN32
	wchar_t* absolutePathHolder = new wchar_t[0x200];
	GetFullPathNameW(path, 0x200, absolutePathHolder, nullptr);
	this->absoluteFilePath = std::shared_ptr<wchar_t>(std::move(absolutePathHolder), std::default_delete<wchar_t[]>());
#endif
}

UnicodeFileReader::~UnicodeFileReader() {

}

FileInputReader::FileInputReader(const char *path) : FileReader(path) {
	fopen_s(&fileHandle, path, "rb+");
	if (fileHandle) {
		fseek(fileHandle, 0x00, SEEK_END);
		setFileSize(_ftelli64(fileHandle));
		rewind(fileHandle);
	}
}

FileInputReader::~FileInputReader() {
	if (fileHandle != nullptr) {
		fclose(fileHandle);
	}
	fileHandle = nullptr;
}

#pragma warning(disable:4996)
UnicodeFileInputReader::UnicodeFileInputReader(const wchar_t *filePath) : UnicodeFileReader(filePath) {
	fileHandle = _wfopen(filePath, L"rb+");
	if (fileHandle) {
		fseek(fileHandle, 0x00, SEEK_END);
		setFileSize(_ftelli64(fileHandle));
		rewind(fileHandle);
	}
}
UnicodeFileInputReader::~UnicodeFileInputReader() {
	if (fileHandle != nullptr) {
		fclose(fileHandle);
	}
	fileHandle = nullptr;
}

char* FileInputReader::readString(const uint64_t bytes) {
	char* buffer = new char[bytes + 1];
	fread(buffer, 1, bytes, fileHandle);
	buffer[bytes] = 0x00;
	addToCaret(bytes);
	return buffer;
}

char* FileInputReader::readString() {
	std::string result = std::string();
	char currentChar = 0x00;
	while ((currentChar = static_cast<char>(readByte())) != 0x00 && getCaret() < getFileSize()) {
		result += currentChar;
	}
	char *dataHolder = new char[result.length() + 1];
	strncpy_s(dataHolder, result.length() + 1, result.c_str(), result.length());
	dataHolder[result.length()] = 0x00;
	return dataHolder;
}

wchar_t* FileInputReader::readUnicodeString(const uint64_t bytes) {
	wchar_t* buffer = new wchar_t[bytes + 1];
	fread(buffer, sizeof(wchar_t), bytes, fileHandle);
	buffer[bytes] = 0x00;
	addToCaret(bytes);
	return buffer;
}

wchar_t* FileInputReader::readUnicodeString() {
	std::wstring result;
	wchar_t currentChar = 0x00;
	while ((currentChar = static_cast<wchar_t>(readUShort())) != 0x00 && getCaret() < getFileSize()) {
		result += currentChar;
	}
	wchar_t *dataHolder = new wchar_t[result.length() + 1];
	wcsncpy_s(dataHolder, result.length() + 1, result.c_str(), result.length());
	dataHolder[result.length()] = 0x00;
	return dataHolder;
}

char* UnicodeFileInputReader::readString(const uint64_t bytes) {
	char* buffer = new char[bytes + 1];
	fread(buffer, 1, bytes, fileHandle);
	buffer[bytes] = 0x00;
	addToCaret(bytes);
	return buffer;
}

char* UnicodeFileInputReader::readString() {
	std::string result = std::string();
	char currentChar = 0x00;
	while ((currentChar = static_cast<char>(readByte())) != 0x00 && getCaret() < getFileSize()) {
		result += currentChar;
	}
	if (currentChar != 0x00) {
		result += currentChar;
	}
	char *dataHolder = new char[result.length() + 1];
	strncpy_s(dataHolder, result.length() + 1, result.c_str(), result.length());
	dataHolder[result.length()] = 0x00;
	return dataHolder;
}

wchar_t* UnicodeFileInputReader::readUnicodeString(const uint64_t bytes) {
	wchar_t* buffer = new wchar_t[bytes + 1];
	fread(buffer, sizeof(wchar_t), bytes, fileHandle);
	buffer[bytes] = 0x00;
	addToCaret(bytes * sizeof(wchar_t));
	return buffer;
}

wchar_t* UnicodeFileInputReader::readUnicodeString() {
	std::wstring result = std::wstring();
	wchar_t currentChar = 0x00;
	
	while ((currentChar = static_cast<wchar_t>(readUShort())) != 0x00 && getCaret() < getFileSize()) {
		result += currentChar;
	}
	if (currentChar != 0x00) {
		result += currentChar;
	}
	wchar_t *dataHolder = new wchar_t[result.length() + 1];
	wcsncpy_s(dataHolder, result.length() + 1, result.c_str(), result.length());
	dataHolder[result.length()] = 0x00;
	return dataHolder;
}