#include "FileWriter.h"
#ifndef _FILEWRITER_USE_WINDOWS_API_
#include <Windows.h>
#endif

FileWriter::FileWriter(const char *path) {
	std::string strPath = std::string(path);
	std::wstring wstrPath = std::wstring(strPath.begin(), strPath.end());
	applyFilePaths(wstrPath.c_str());
}

FileWriter::FileWriter(const wchar_t *path) {
	applyFilePaths(path);
}

FileWriter::~FileWriter() {

}

void FileWriter::applyFilePaths(const wchar_t* path) {
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

void FileWriter::close() {

}

FileInputWriter::FileInputWriter(const char* path) : FileInputWriter(path, false) {
}

FileInputWriter::FileInputWriter(const char* path, bool appendData) : FileWriter(path) {
}

FileInputWriter::FileInputWriter(const wchar_t* path) : FileInputWriter(path, false) {

}
FileInputWriter::FileInputWriter(const wchar_t* path, bool appendData) : FileWriter(path) {
	openFile(appendData);
}

void FileInputWriter::openFile(bool appendData) {
#ifdef _FILEWRITER_USE_WINDOWS_API_
	fileHandle = CreateFile(getFilePath().get(), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#else
	fileHandle = _wfopen(getFilePath().get(), appendData ? L"a+" : L"wb+");
#endif
}

FileInputWriter::~FileInputWriter() {
	close();
}
void FileInputWriter::close() {
	if (fileHandle) {
#ifdef _FILEWRITER_USE_WINDOWS_API_
		CloseHandle(fileHandle);
#else
		fclose(fileHandle);
#endif
		fileHandle = nullptr;
	}
}
inline void FileInputWriter::writeULong(size_t data) {
	write<size_t>(data);
}
inline void FileInputWriter::writeUInt(uint32_t data) {
	write<uint32_t>(data);
}
inline void FileInputWriter::writeUShort(uint16_t data) {
	write<uint16_t>(data);
}
inline void FileInputWriter::writeByte(uint8_t data) {
	write<uint8_t>(data);
}
inline void FileInputWriter::writeFloat(float data) {
	write<float>(data);
}
inline void FileInputWriter::writeDouble(double data) {
	write<double>(data);
}

inline void FileInputWriter::writeString(const char* str) {
	FileWriter::writeString(str);
}
inline void FileInputWriter::writeString(const char* str, size_t length) {
	writePtr<const char*>(str, length);
}
inline void FileInputWriter::writeUnicodeString(const wchar_t* str) {
	FileWriter::writeUnicodeString(str);
}
inline void FileInputWriter::writeUnicodeString(const wchar_t* str, uint64_t length) {
	writePtr<const wchar_t*>(str, length);
}