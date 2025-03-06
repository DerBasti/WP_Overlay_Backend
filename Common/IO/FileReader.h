#ifndef __ROSE_FILE_READER__
#define __ROSE_FILE_READER__
#pragma once

#include <iostream>
#include <memory>
#include <type_traits>

class DataReader {
private:
	uint64_t caret;
	uint64_t size;
protected:
	virtual void onCaretBytesSkipped(const uint64_t bytesSkipped) {

	}
	virtual void onCaretAdd(const uint64_t caretPosition, const uint64_t bytesToAdd) {

	}
	virtual void onCaretReset(const uint64_t newCaretPosition) {

	}

	__inline void setFileSize(const uint64_t size) {
		this->size = size;
	}

	virtual void resetCaretToPosition(const uint64_t caretPosition) {
		onCaretReset(caretPosition);
		caret = caretPosition;
	}

	virtual void addToCaret(const uint64_t amountOfBytes) {
		onCaretAdd(caret, amountOfBytes);
		caret += amountOfBytes;
	}

	virtual void skipBytesForCaret(const uint64_t amountOfBytes) {
		onCaretBytesSkipped(amountOfBytes);
		caret += amountOfBytes;
	}
public:
	DataReader() {
		caret = size = 0;
	}
	virtual ~DataReader() {}

	__inline virtual void skipBytes(const uint64_t amountOfBytes) {
		skipBytesForCaret(amountOfBytes);
	}

	__inline uint64_t getCaret() const {
		return caret;
	}

	virtual bool isEndOfFile() const {
		return true;
	}

	virtual bool isValid() const {
		return this->operator bool();
	}

	virtual operator bool() const {
		return false;
	}

	inline virtual uint64_t readULong() {
		return 0;
	}
	__inline virtual uint32_t readUInt() {
		return 0;
	}
	__inline virtual uint16_t readUShort() {
		return 0;
	}
	__inline virtual uint8_t readByte() {
		return 0;
	}
	__inline virtual float readFloat() {
		return 0.0f;
	}
	__inline virtual double readDouble() {
		return 0.0;
	}
	__inline uint64_t getFileSize() const {
		return size;
	}
	std::shared_ptr<char> readStringWrapped(const uint64_t bytes) {
		return std::shared_ptr<char>(readString(bytes), std::default_delete<char[]>());
	}
	std::shared_ptr<char> readStringWrapped() {
		return std::shared_ptr<char>(readString(), std::default_delete<char[]>());
	}
	std::shared_ptr<wchar_t> readUnicodeStringWrapped(const uint64_t bytes) {
		return std::shared_ptr<wchar_t>(readUnicodeString(bytes), std::default_delete<wchar_t[]>());
	}
	std::shared_ptr<wchar_t> readUnicodeStringWrapped() {
		return std::shared_ptr<wchar_t>(readUnicodeString(), std::default_delete<wchar_t[]>());
	}
	virtual char* readString(const uint64_t bytes) {
		return nullptr;
	}
	virtual char* readString() {
		return nullptr;
	}
	virtual wchar_t* readUnicodeString(const uint64_t bytes) {
		return nullptr;
	}
	virtual wchar_t* readUnicodeString() {
		return nullptr;
	}
};

class LoadedDataReader : public DataReader {
private:
	const char* dataHolder;
	template<class _T>
	const _T readNumerical() {
		const _T result = *reinterpret_cast<const _T*>(dataHolder);
		dataHolder += sizeof(_T);
		addToCaret(sizeof(_T));
		return result;
	}
protected:
	virtual void onCaretBytesSkipped(const uint64_t bytesSkipped) {
		dataHolder += bytesSkipped;
	}
public:
	LoadedDataReader(const char* readData) {
		dataHolder = readData;
	}
	virtual ~LoadedDataReader() {

	}
	inline virtual uint64_t readULong() {
		return readNumerical<uint64_t>();
	}
	__inline virtual uint32_t readUInt() {
		return readNumerical<uint32_t>();
	}
	__inline virtual uint16_t readUShort() {
		return readNumerical<uint16_t>();
	}
	__inline virtual uint8_t readByte() {
		return readNumerical<uint8_t>();
	}
	__inline virtual float readFloat() {
		return readNumerical<float>();
	}
	__inline virtual double readDouble() {
		return readNumerical<double>();
	}
	virtual char* readString(const size_t bytes) {
		char* result = new char[bytes + 1];
		memcpy(result, dataHolder, bytes);
		result[bytes] = 0x00;
		addToCaret(bytes);
		return result;
	}
	virtual char* readString() {
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
	virtual wchar_t* readUnicodeString(const uint64_t bytes) {
		wchar_t* result = new wchar_t[bytes + 1];
		memcpy(result, dataHolder, bytes * sizeof(wchar_t));
		result[bytes] = 0x00;
		addToCaret(bytes * sizeof(wchar_t));
		return result;
	}
	virtual wchar_t* readUnicodeString() {
		std::wstring result = std::wstring();
		wchar_t currentChar = 0x00;
		while ((currentChar = static_cast<wchar_t>(this->readUShort())) != 0x00 && getCaret() < getFileSize()) {
			result += currentChar;
		}
		wchar_t *dataHolder = new wchar_t[result.length() + 1];
		wcsncpy_s(dataHolder, result.length() + 1, result.c_str(), result.length());
		dataHolder[result.length()] = 0x00;
		return dataHolder;
	}
};

class FileReader : public DataReader {
private:
	std::shared_ptr<char> filePath;
	std::shared_ptr<char> absoluteFilePath;
public:
	FileReader(const char *path);
	virtual ~FileReader();

	virtual void resetCaretTo(const uint64_t caretPosition) {
		resetCaretToPosition(caretPosition);
	}

	virtual std::shared_ptr<char> getFilePath() const {
		return filePath;
	}
	virtual std::shared_ptr<char> getAbsoluteFilePath() const {
		return absoluteFilePath;
	}
};

class UnicodeFileReader : public DataReader {
private:
	std::shared_ptr<wchar_t> filePath;
	std::shared_ptr<wchar_t> absoluteFilePath;
public:
	UnicodeFileReader(const wchar_t *path);
	virtual ~UnicodeFileReader();

	virtual void resetCaretTo(const uint64_t caretPosition) {
		resetCaretToPosition(caretPosition);
	}

	virtual std::shared_ptr<wchar_t> getFilePath() const {
		return filePath;
	}
	virtual std::shared_ptr<wchar_t> getAbsoluteFilePath() const {
		return absoluteFilePath;
	}
};

class FileInputReader : public FileReader {
private:
	FILE * fileHandle;
protected:
	template<class _T, class = typename std::enable_if<std::is_integral<_T>::value || std::is_floating_point<_T>::value>::type>
	_T read() {
		_T value;
		uint32_t typeSize = sizeof(_T);
		fread(&value, typeSize, 1, fileHandle);
		addToCaret(typeSize);
		return value;
	}

	virtual void onCaretBytesSkipped(const uint64_t bytesSkipped) {
		_fseeki64(fileHandle, bytesSkipped, SEEK_CUR);
	}
	virtual void onCaretReset(const uint64_t caret) {
		_fseeki64(fileHandle, caret, SEEK_SET);
	}
	virtual void onCaretAdd(const uint64_t caretPosition, const uint64_t bytesToAdd) {
	}
public:
	FileInputReader(const char *filePath);
	virtual ~FileInputReader();

	inline virtual uint64_t readULong() {
		return read<uint64_t>();
	}
	__inline virtual uint32_t readUInt() {
		return read<uint32_t>();
	}
	__inline virtual uint16_t readUShort() {
		return read<uint16_t>();
	}
	__inline virtual uint8_t readByte() {
		return read<uint8_t>();
	}
	__inline virtual float readFloat() {
		return read<float>();
	}
	__inline virtual double readDouble() {
		return read<double>();
	}

	virtual operator bool() const {
		return fileHandle != nullptr;
	}

	virtual char* readString(const uint64_t bytes);
	virtual char* readString();
	virtual wchar_t* readUnicodeString(const uint64_t bytes);
	virtual wchar_t* readUnicodeString();

	__inline virtual bool isEndOfFile() const {
		return fileHandle == nullptr || getCaret() >= getFileSize();
	}
};

class UnicodeFileInputReader : public UnicodeFileReader {
private:
	FILE * fileHandle;
protected:
	template<class _T, class = typename std::enable_if<std::is_integral<_T>::value || std::is_floating_point<_T>::value>::type>
	_T read() {
		_T value;
		uint32_t typeSize = sizeof(_T);
		fread(&value, typeSize, 1, fileHandle);
		addToCaret(typeSize);
		return value;
	}

	virtual void onCaretBytesSkipped(const uint64_t bytesSkipped) {
		_fseeki64(fileHandle, bytesSkipped, SEEK_CUR);
	}
	virtual void onCaretReset(const uint64_t caret) {
		_fseeki64(fileHandle, caret, SEEK_SET);
	}
	virtual void onCaretAdd(const uint64_t caretPosition, const uint64_t bytesToAdd) {
	}
public:
	UnicodeFileInputReader(const wchar_t *filePath);
	virtual ~UnicodeFileInputReader();

	inline virtual uint64_t readULong() {
		return read<uint64_t>();
	}
	__inline virtual uint32_t readUInt() {
		return read<uint32_t>();
	}
	__inline virtual uint16_t readUShort() {
		return read<uint16_t>();
	}
	__inline virtual uint8_t readByte() {
		return read<uint8_t>();
	}
	__inline virtual float readFloat() {
		return read<float>();
	}
	__inline virtual double readDouble() {
		return read<double>();
	}

	virtual operator bool() const {
		return fileHandle != nullptr;
	}

	virtual char* readString(const uint64_t bytes);
	virtual char* readString();
	virtual wchar_t* readUnicodeString();
	virtual wchar_t* readUnicodeString(const uint64_t bytes);

	__inline virtual bool isEndOfFile() const {
		return fileHandle == nullptr || getCaret() >= getFileSize();
	}
};


#endif //__ROSE_FILE_READER__
