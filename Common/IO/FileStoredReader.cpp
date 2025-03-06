#include "FileStoredReader.h"

FileStoredReader::FileStoredReader(const char *path) : FileReader(path) {
	FileInputReader reader(path);
	if (reader.isValid()) {
		fileInBytes = reader.readString(reader.getFileSize());
		iterator = fileInBytes;
		setFileSize(reader.getFileSize());
	}
	else {
		fileInBytes = iterator = nullptr;
		setFileSize(0);
	}
}
FileStoredReader::~FileStoredReader() {
	if (fileInBytes != nullptr) {
		delete[] fileInBytes;
		fileInBytes = nullptr;
	}
	iterator = nullptr;
}

char* FileStoredReader::readString(const uint64_t bytes) {
	char *string = new char[bytes + 1];
	memcpy(string, (void*)iterator, bytes);
	string[bytes] = 0x00;
	addToCaret(bytes);
	return string;
}