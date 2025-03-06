#include "FileListener.h"
#include "../Logging/CrashHandler.h"
#include "../Logging/ErrorCodeTranslator.h"
#include <Windows.h>

FileListener::FileListener(const wchar_t* path) {
	this->path = std::wstring(path);
	if (this->path.at(this->path.length() - 1) != '\\' && this->path.at(this->path.length()-1) != '/') {
		this->path += std::wstring(L"/");
	}
	listeningAllowed = false;
}

FileListener::~FileListener() {
	stopListening();
}

void FileListener::startListening() {
	if (listeningAllowed) {
		logger.logDebug("FileListener was requested to start, but is already running.");
		return;
	}
	logger.logDebug("Starting FileListener for path: ", path.c_str());
	listeningAllowed = true;
	this->listenerThread = std::thread([&]() {
		//CoInitialize(nullptr);
		CrashHandler::AttachSignalHandler();
		HANDLE directoryHandle = CreateFile(getPath().c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);
		uint8_t buffer[1024] = { 0x00 };
		FILE_NOTIFY_INFORMATION* ptr;
		DWORD bytesWritten = 0;
		OVERLAPPED overlapped;
		overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
		if (!overlapped.hEvent) {
			logger.logError("CreateEvent for FileListener failed. No listening/notification will be done. ErrorReason: ", ErrorCodeTranslator::GetErrorCodeString().c_str());
			CloseHandle(directoryHandle);
			return;
		}
		BOOL readResult = ReadDirectoryChangesW(directoryHandle, buffer, 1024, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, nullptr, &overlapped, nullptr);
		while (listeningAllowed) {
			DWORD result = WaitForSingleObject(overlapped.hEvent, 250);
			if (result != WAIT_OBJECT_0) {
				continue;
			}
			ptr = (FILE_NOTIFY_INFORMATION*)buffer;
			do {
				ptr->FileName[ptr->FileNameLength / 2] = 0x00;
				FileListenerEventType type = FileListenerEventType::INVALID;
				switch (ptr->Action) {
				case FILE_ACTION_MODIFIED:
					type = FileListenerEventType::FILE_WRITTEN;
					break;
				case FILE_ACTION_ADDED:
					type = FileListenerEventType::FILE_CREATED;
					break;
				case FILE_ACTION_REMOVED:
					type = FileListenerEventType::FILE_DELETED;
					break;
				}
				FileListenerEvent event(getPath().c_str(), ptr->FileName, type);
				onFileModified(&event);
				*((uint8_t**)&ptr) += ptr->NextEntryOffset;
			} while (ptr->NextEntryOffset);
			readResult = ReadDirectoryChangesW(directoryHandle, buffer, 1024, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, &bytesWritten, &overlapped, nullptr);
		}
		CloseHandle(overlapped.hEvent);
		CloseHandle(directoryHandle);
		//CoUninitialize();
	});
}

void FileListener::stopListening() {
	logger.logDebug("Stopping FileListener from listening.");
	listeningAllowed = false;
	if (listenerThread.native_handle()) {
		while (!listenerThread.joinable()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		listenerThread.join();
	}
	logger.logDebug("FileListener successfully stopped.");
}

void FileListener::onFileModified(FileListenerEvent* event) {
	logger.logInfo("FileListener Notification: [", event->getEventType(), " in Directory: ", event->getDirectory().c_str(), ", File ", event->getFileName().c_str(), "]");
	auto listenerIt = onFileModifiedListener.find(event->getFileName());
	if (listenerIt == onFileModifiedListener.cend()) {
		listenerIt = onFileModifiedListener.find(event->getDirectory().c_str());
	}
	if (listenerIt != onFileModifiedListener.cend()) {
		auto& listenerVector = listenerIt->second;
		for (auto listener : listenerVector) {
			listener(event);
		}
	}
}

std::ostream& operator<<(std::ostream& out, FileListenerEventType eventType) {
	static std::unordered_map<FileListenerEventType, const char*> EVENT_TYPES{
		{FileListenerEventType::INVALID, "INVALID"},
		{FileListenerEventType::FILE_WRITTEN, "FILE_WRITTEN"},
		{FileListenerEventType::FILE_DELETED, "FILE_DELETED"},
		{FileListenerEventType::FILE_CREATED, "FILE_CREATED"}
	};
	out << EVENT_TYPES[eventType];
	return out;
}

std::wostream& operator<<(std::wostream& out, FileListenerEventType eventType) {
	static std::unordered_map<FileListenerEventType, const char*> EVENT_TYPES{
		{FileListenerEventType::INVALID, "INVALID"},
		{FileListenerEventType::FILE_WRITTEN, "FILE_WRITTEN"},
		{FileListenerEventType::FILE_DELETED, "FILE_DELETED"},
		{FileListenerEventType::FILE_CREATED, "FILE_CREATED"}
	};
	out << EVENT_TYPES[eventType];
	return out;
}