#ifndef __FILE_WRITER__
#define __FILE_WRITER__

#ifndef _FILEWRITER_USE_WINDOWS_API_
	#define _FILEWRITER_USE_WINDOWS_API_
#endif //_FILEWRITER_USE_WINDOWS_API_

#include <inttypes.h>
#include <string>
#include <memory>
#ifdef _FILEWRITER_USE_WINDOWS_API_
#include <Windows.h>
#endif

class DataWriter {
private:
	uint64_t caret;
protected:
	virtual void addToCaret(const uint64_t amountOfBytes) {
		onCaretAdd(caret, amountOfBytes);
		caret += amountOfBytes;
	}
	virtual void onCaretAdd(const uint64_t caretPosition, const uint64_t bytesToAdd) {

	}
public:
	DataWriter() {
		caret = 0;
	}
	virtual ~DataWriter() {

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
	virtual void writeULong(size_t data) {

	}
	virtual void writeUInt(uint32_t data) {

	}
	virtual void writeUShort(uint16_t data) {

	}
	virtual void writeByte(uint8_t data) {

	}
	virtual void writeFloat(float data) {

	}
	virtual void writeDouble(double data) {

	}
	virtual void writeString(const char* str) {
		writeString(str, str ? strlen(str) : 0);
	}
	virtual void writeString(const char* str, uint64_t length) {

	}
	virtual void writeUnicodeString(const wchar_t* str) {
		writeUnicodeString(str, str ? wcslen(str) : 0);
	}
	virtual void writeUnicodeString(const wchar_t* str, uint64_t length) {

	}
};

class FileWriter : public DataWriter {
private:
	std::shared_ptr<wchar_t> filePath;
	std::shared_ptr<wchar_t> absoluteFilePath;

	void applyFilePaths(const wchar_t* path);
public:
	FileWriter(const char *path);
	FileWriter(const wchar_t *path);
	virtual ~FileWriter();
	virtual void close();

	inline std::shared_ptr<wchar_t> getFilePath() const {
		return filePath;
	}
	inline std::shared_ptr<wchar_t> getAbsoluteFilePath() const {
		return absoluteFilePath;
	}
};

#pragma warning(disable:4996)
class FileInputWriter : public FileWriter {
private:
#ifdef _FILEWRITER_USE_WINDOWS_API_
	HANDLE fileHandle;
#else
	FILE *fileHandle;
#endif
	void openFile(bool appendData);
protected:
	template<class _Type>
	inline void write(const _Type& value) {
		write<_Type>(value, sizeof(_Type));
	}

	template<class _Type, class = typename std::enable_if<std::is_pointer_v<_Type> == false>::type>
	inline void write(const _Type& value, size_t length) {
#ifdef _FILEWRITER_USE_WINDOWS_API_
		WriteFile(fileHandle, &value, (DWORD)length, nullptr, nullptr);
#else
		fwrite(&value, length, 1, fileHandle);
#endif
		addToCaret(length);
	}

	template<class _Type, class _TypeValuable = std::remove_pointer_t<_Type>, class = typename std::enable_if<std::is_pointer_v<_Type>>::type>
	inline void writePtr(const _Type& value) {
#ifdef _FILEWRITER_USE_WINDOWS_API_
		WriteFile(fileHandle, value, sizeof(_TypeValuable), nullptr, nullptr);
#else
		fwrite(value, 1, sizeof(_TypeValuable), fileHandle);
#endif
		addToCaret(sizeof(_TypeValuable));
	}

	template<class _Type, class _TypeValuable = std::remove_pointer_t<_Type>, class = typename std::enable_if<std::is_pointer_v<_Type>>::type>
	inline void writePtr(const _Type& value, size_t length) {
#ifdef _FILEWRITER_USE_WINDOWS_API_
		WriteFile(fileHandle, value, sizeof(_TypeValuable)*(DWORD)length, nullptr, nullptr);
#else 
		fwrite(value, length, sizeof(_TypeValuable), fileHandle);
#endif
		addToCaret(length * sizeof(_TypeValuable));
	}
public:
	FileInputWriter(const char* path);
	FileInputWriter(const char* path, bool appendData);
	FileInputWriter(const wchar_t* path);
	FileInputWriter(const wchar_t* path, bool appendData);
	virtual ~FileInputWriter();
	virtual void close();
	virtual operator bool() const {
		return fileHandle != nullptr;
	}
	virtual void writeULong(size_t data);
	virtual void writeUInt(uint32_t data);
	virtual void writeUShort(uint16_t data);
	virtual void writeByte(uint8_t data);
	virtual void writeFloat(float data);
	virtual void writeDouble(double data);
	virtual void writeString(const char* str);
	virtual void writeString(const char* str, size_t length);
	virtual void writeUnicodeString(const wchar_t* str);
	virtual void writeUnicodeString(const wchar_t* str, uint64_t length);
};



#endif //__FILE_WRITER__