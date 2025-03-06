#ifndef __FILE_LISTENER__
#define __FILE_LISTENER__

#include <string>
#include <functional>
#include <vector>
#include <thread>
#include <unordered_map>
#include "../Logging/Logger.h"

enum class FileListenerEventType {
	INVALID,
	FILE_WRITTEN,
	FILE_DELETED,
	FILE_CREATED
};

std::ostream& operator<<(std::ostream& out, FileListenerEventType eventType);
std::wostream& operator<<(std::wostream& out, FileListenerEventType eventType);

class FileListenerEvent {
private:
	std::wstring path;
	std::wstring fileName;
	FileListenerEventType type;
public:
	FileListenerEvent(const wchar_t* path, const wchar_t* fileName, FileListenerEventType type) {
		this->path = std::wstring(path);
		this->fileName = std::wstring(fileName);
		this->type = type;
	}
	virtual ~FileListenerEvent() {

	}
	__inline std::wstring getDirectory() const {
		return path;
	}
	__inline std::wstring getFileName() const {
		return fileName;
	}
	__inline std::wstring getFullPath() const {
		return path + fileName;
	}
	__inline FileListenerEventType getEventType() const {
		return type;
	}
};

class FileListener {
private:
	ROSEThreadedLogger logger;
	std::thread listenerThread;
	bool listeningAllowed;
	std::wstring path;
	std::unordered_map<std::wstring, std::vector<std::function<void(FileListenerEvent*)>>> onFileModifiedListener;
protected:
	virtual void onFileModified(FileListenerEvent* event);
public:
	FileListener() : FileListener(L"") {}
	FileListener(const wchar_t* path);
	virtual ~FileListener();

	void startListening();
	void stopListening();

	void addOnFileModifiedListener(std::wstring filePathToListenFor, std::function<void(FileListenerEvent*)> listener) {
		logger.logInfo("Adding FileModificationListener for file path: ", filePathToListenFor.c_str());
		std::vector<std::function<void(FileListenerEvent*)>> vector;
		auto it = onFileModifiedListener.find(filePathToListenFor);
		if (it != onFileModifiedListener.cend()) {
			vector = it->second;
		}
		vector.push_back(listener);
		onFileModifiedListener.insert_or_assign(filePathToListenFor, std::move(vector));
	}
	std::wstring getPath() const {
		return path;
	}
};

#endif 