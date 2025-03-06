#ifndef __ROSE_FILESTOREDREADER__
#define __ROSE_FILESTOREDREADER__

#include "FileReader.h"

class FileStoredReader : public FileReader {
private:
	const char *fileInBytes;
	const char *iterator;
protected:
	template<class _T, class = typename std::enable_if<std::is_integral<_T>::value || std::is_floating_point<_T>::value>::type>
	_T read() {
		_T* currentDataAsPointer = (_T*)iterator;
		_T value = *currentDataAsPointer;
		addToCaret(sizeof(_T));
		return value;
	}
	virtual void onCaretAdd(const uint64_t caret, const uint64_t bytesToAdd) {
		iterator += bytesToAdd;
	}
	virtual void onCaretBytesSkipped(const uint64_t bytesToAdd) {
		iterator += bytesToAdd;
	}
	virtual void onCaretReset(const uint64_t newCaretPosition) {
		iterator = fileInBytes + newCaretPosition;
	}
public:
	FileStoredReader(const char* path);
	virtual ~FileStoredReader();

	virtual operator bool() const {
		return fileInBytes != nullptr;
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

	virtual char* readString(const uint64_t bytesToRead);

	__inline virtual bool isEndOfFile() const {
		return fileInBytes == nullptr || getCaret() >= getFileSize();
	}
};

#endif //__ROSE_FILESTOREDREADER__
