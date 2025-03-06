#ifndef __PROCESS_WATCHER__
#define __PROCESS_WATCHER__

#include <Windows.h>
#include <Psapi.h>
#include <DbgHelp.h>
#include <tlhelp32.h>
#include <inttypes.h>
#include <memory>
#include <vector>
#include <string>

class Process {
private:
	uint32_t processId;
	std::shared_ptr<wchar_t> processName;
	std::shared_ptr<wchar_t> filePath;
	std::vector<std::wstring> startParameter;
	HANDLE processHandle;
public:
	Process() : Process(nullptr, L"", L"", 0, std::vector<std::wstring>()) {}
	Process(HANDLE handle, const wchar_t *exeName, const wchar_t* filePath, uint32_t processId, std::vector<std::wstring> params) {
		this->processHandle = handle;
		this->processId = processId;
		this->startParameter = params;
		size_t len = exeName != nullptr ? wcslen(exeName) : 0;
		processName = std::shared_ptr<wchar_t>(new wchar_t[len + 1], std::default_delete<wchar_t[]>());
		wcsncpy_s(processName.get(), len + 1, (exeName ? exeName : L""), len);

		size_t pathLen = filePath != nullptr ? wcslen(filePath) : 0;
		this->filePath = std::shared_ptr<wchar_t>(new wchar_t[pathLen + 1], std::default_delete<wchar_t[]>());
		wcsncpy_s(this->filePath.get(), pathLen + 1, (filePath ? filePath : L""), pathLen);

		processName.get()[len] = 0x00;
	}
	Process& operator=(const Process& p) {
		if (this->processHandle) {
			CloseHandle(processHandle);
		}
		processId = p.getProcessId();
		size_t newExeNameLength = p.getProcessExeName() ? wcsnlen(p.getProcessExeName().get(), -1) : 0;
		if (wcsnlen(getProcessExeName().get(), -1) < newExeNameLength) {
			processName = std::shared_ptr<wchar_t>(new wchar_t[newExeNameLength + 1], std::default_delete<wchar_t[]>());
		}
		wcsncpy_s(processName.get(), newExeNameLength + 1, p.getProcessExeName().get(), newExeNameLength);

		size_t pathLen = filePath != nullptr ? wcslen(p.getProcessFilePath().get()) : 0;
		if (wcsnlen(getProcessFilePath().get(), -1) < pathLen) {
			filePath = std::shared_ptr<wchar_t>(new wchar_t[pathLen + 1], std::default_delete<wchar_t[]>());
		}
		wcsncpy_s(filePath.get(), pathLen + 1, p.getProcessFilePath().get(), pathLen);
		startParameter.clear();
		for (auto param : p.getStartParameters()) {
			startParameter.push_back(param);
		}
		processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processId);
		return (*this);
	}
	virtual ~Process() {
		processName = nullptr;
		filePath = nullptr;
		if (processHandle) {
			CloseHandle(processHandle);
		}
		processHandle = nullptr;
	}
	__inline HANDLE getHandle() const {
		return processHandle;
	}
	__inline uint32_t getProcessId() const {
		return processId;
	}
	__inline std::shared_ptr<wchar_t> getProcessExeName() const {
		return processName;
	}
	__inline std::shared_ptr<wchar_t> getProcessFilePath() const {
		return filePath;
	}
	inline const std::vector<std::wstring>& getStartParameters() const {
		return startParameter;
	}
		
	bool isValid() const;
};

class ProcessWatcher {
private:
	ProcessWatcher() {}
	static std::vector<std::wstring> ExtractCommandlineFromProcessHandle(HANDLE handle);
	static Process GetProcess(const wchar_t *exeName, bool readStartParameter);
public:
	static bool IsProcessRunning(const wchar_t *exeName) {
		return ProcessWatcher::GetProcess(exeName).isValid();
	}

	inline static Process GetProcess(const wchar_t *exeName) {
		return GetProcess(exeName, false);
	}
	static Process GetProcessWithStartParameters(const wchar_t* exeName) {
		return GetProcess(exeName, true);
	}
	static Process GetProcessByWindowName(const wchar_t* windowName);
	inline static HWND GetWindowByProcess(const Process& process) {
		return GetWindowByProcess(process, 0, 0);
	}
	static Process GetProcessByProcessId(uint32_t processId) {
		return GetProcessWithStartParametersByProcessId(processId, false);
	}
	static Process GetProcessWithStartParametersByProcessId(uint32_t processId, bool getStartParameterFlag);
	inline static HWND GetWindowByProcess(const Process& process, uint32_t minWindowWidth, uint32_t minWindowHeight) {
		return GetWindowByProcess(process.getProcessId(), minWindowWidth, minWindowHeight);
	}
	static HWND GetWindowByProcess(uint32_t processId, uint32_t minWindowWidth, uint32_t minWindowHeight);

	inline static HWND GetWindowByProcessUntilVisible(const Process& process, uint32_t minWindowWidth, uint32_t minWindowHeight) {
		return GetWindowByProcessUntilVisible(process.getProcessId(), minWindowWidth, minWindowHeight);
	}
	static size_t GetProcessModuleBaseAddress(const Process& process);
	static HWND GetWindowByProcessUntilVisible(uint32_t processId, uint32_t minWindowWidth, uint32_t minWindowHeight);
	static bool StartProcessFrom(const wchar_t *totalPath);
	static bool StartProcessFrom(const wchar_t *path, const wchar_t* exeName);
	static void KillProcess(Process& p);
};

#endif //__PROCESS_WATCHER__