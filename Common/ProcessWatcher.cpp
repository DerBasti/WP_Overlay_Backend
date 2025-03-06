#include "ProcessWatcher.h"
#include <process.h>
#include <winternl.h>
#include <stack>
#include <Windows.h>
#include "./Logging/Logger.h"

#pragma comment(lib, "ntdll.lib")

bool procRealized = false;

bool Process::isValid() const {
	if (getHandle() == nullptr || getProcessId() == 0) {
		return false;
	}
	auto process = ProcessWatcher::GetProcess(getProcessExeName().get());
	auto pId = GetProcessId(process.getHandle());
	return getHandle() != nullptr && processId == pId;
}

Process ProcessWatcher::GetProcess(const wchar_t *exeName, bool readStartParameter) {
	if (exeName == nullptr || wcslen(exeName) == 0) {
		return Process();
	}
	wchar_t entireFilePath[0x200] = { 0x00 };
	std::wstring directory;
	HANDLE processHandle = nullptr;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 entry = { 0x00 };
	entry.dwSize = sizeof(PROCESSENTRY32);
	std::vector<std::wstring> parameters;
	uint32_t processId = 0;
	if (Process32First(snapshot, &entry) == TRUE) {
		do {
			if (_wcsicmp(entry.szExeFile, exeName) == 0) {
				processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, entry.th32ProcessID);  
				
				if (readStartParameter) {
					parameters = ProcessWatcher::ExtractCommandlineFromProcessHandle(processHandle);
				}

				if (!GetModuleFileNameEx(processHandle, nullptr, entireFilePath, 0x200)) {
					CloseHandle(processHandle);
					return Process();
				} 
				std::wstring wstrPath = std::wstring(entireFilePath);
				directory = wstrPath.substr(0, wstrPath.find_last_of('\\') + 1);
				processId = entry.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &entry) == TRUE);
	}
	CloseHandle(snapshot);
	return Process(processHandle, exeName, directory.c_str(), processId, parameters);
}

size_t ProcessWatcher::GetProcessModuleBaseAddress(const Process& p) {
	std::shared_ptr<HMODULE> hMods = std::shared_ptr<HMODULE>(new HMODULE[2048], std::default_delete<HMODULE[]>());
	DWORD cbNeeded;
	ULONG64 result = 0;
	if (EnumProcessModules(p.getHandle(), hMods.get(), sizeof(hMods), &cbNeeded)) {
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			wchar_t szModName[MAX_PATH];
			if (GetModuleFileNameEx(p.getHandle(), hMods.get()[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
				std::wstring wstrModName = szModName;
				wstrModName = wstrModName.substr(wstrModName.find_last_of('\\')+1);
				if (_wcsicmp(p.getProcessExeName().get(), wstrModName.c_str()) == 0) {
					return (size_t)hMods.get()[i];
				}
			}
		}
	}
	return (std::numeric_limits<size_t>::max)();
}

Process ProcessWatcher::GetProcessWithStartParametersByProcessId(uint32_t processId, bool readStartParameter) {
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
	if (!processHandle) {
		return Process();
	}
	std::vector<std::wstring> parameters;
	if (readStartParameter) {
		parameters = ProcessWatcher::ExtractCommandlineFromProcessHandle(processHandle);
	}
	wchar_t buffer[0x200] = { 0x00 };
	if (!GetModuleFileNameEx(processHandle, nullptr, buffer, 0x1FF)) {
		CloseHandle(processHandle);
		return Process();
	}
	std::wstring filePath = std::wstring(buffer);
	std::wstring exeName = filePath.substr(filePath.find_last_of(L"\\") + 1);
	filePath = filePath.substr(0, filePath.find_last_of(L"\\")+1);
	return Process(processHandle, exeName.c_str(), filePath.c_str(), processId, parameters);
}

std::vector<std::wstring> ProcessWatcher::ExtractCommandlineFromProcessHandle(HANDLE handle) {
	if (!handle) {
		return std::vector<std::wstring>();
	}
	ROSEThreadedLogger logger;
	PROCESS_BASIC_INFORMATION pbi{ 0 };
	ULONG len = 0;
	NtQueryInformationProcess(handle, ProcessBasicInformation, &pbi, sizeof(pbi), &len);

#ifndef ProcessCommandLineInformation
#define ProcessCommandLineInformation (PROCESSINFOCLASS)60
#endif

	/*
	PVOID rtlUserProcParamsAddress;
	ReadProcessMemory(handle,
		&(((_PEB*)pbi.PebBaseAddress)->ProcessParameters),
		&rtlUserProcParamsAddress,
		sizeof(PVOID), NULL);

	UNICODE_STRING commandLine;
	ReadProcessMemory(handle,
		&(((_RTL_USER_PROCESS_PARAMETERS*)rtlUserProcParamsAddress)->CommandLine),
		&commandLine, sizeof(commandLine), NULL);

	wchar_t commandLineContents[0x1000] = { 0x00 };
	ReadProcessMemory(handle, commandLine.Buffer,
		commandLineContents, commandLine.Length, NULL);
		*/
	NtQueryInformationProcess(handle, ProcessCommandLineInformation, nullptr, 0, &len);
	std::vector<BYTE> commandLineBytes(len);
	NtQueryInformationProcess(handle, ProcessCommandLineInformation, commandLineBytes.data(), len, nullptr);
	PUNICODE_STRING commandLineArgs = reinterpret_cast<PUNICODE_STRING>(commandLineBytes.data());
	std::wstring contents;
	contents.assign(commandLineArgs->Buffer, commandLineArgs->Length / sizeof(WCHAR));
	logger.logDebug("Found process parameters: ", contents.c_str());
	//std::wstring contents = std::wstring(commandLineContents);
	std::wstring currentParam;
	bool quoteFound = false;
	std::vector<std::wstring> parameters;
	for (auto it = contents.begin(); it != contents.end(); it++) {
		wchar_t currentChar = (*it);
		if (currentChar == '\"') {
			quoteFound = !quoteFound;
		}
		currentParam += currentChar;
		if (currentChar == ' ' && !quoteFound) {
			parameters.push_back(currentParam);
			currentParam.clear();
		}
	}
	return parameters;
}

Process ProcessWatcher::GetProcessByWindowName(const wchar_t* windowName) {
	HWND window = FindWindowW(nullptr, windowName);
	if (!window) {
		return Process();
	}
	DWORD processId = 0;
	GetWindowThreadProcessId(window, &processId); 
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, processId);
	wchar_t entireFilePath[0x200] = { 0x00 };
	GetModuleFileNameEx(processHandle, nullptr, entireFilePath, 0x200);
	std::wstring wstrPath = std::wstring(entireFilePath);
	std::wstring directory = wstrPath.substr(0, wstrPath.find_last_of('\\') + 1);
	std::wstring exeName = wstrPath.substr(wstrPath.find_last_of('\\') + 1);
	return Process(processHandle, exeName.c_str(), directory.c_str(), processId, std::vector<std::wstring>());
}

struct WindowAndThreadStruct {
	HWND windowHandle;
	uint32_t processId;
	uint32_t minWidth;
	uint32_t minHeight;
};

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	WindowAndThreadStruct* process = (WindowAndThreadStruct*)lParam;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	RECT windowRect;
	GetClientRect(hwnd, &windowRect);
	uint32_t width = windowRect.right - windowRect.left;
	uint32_t height = windowRect.bottom - windowRect.top;
	if (lpdwProcessId == process->processId && width > process->minWidth && height > process->minHeight)
	{
		process->windowHandle = hwnd;
		return FALSE;
	}
	return TRUE;
}

HWND ProcessWatcher::GetWindowByProcess(uint32_t processId, uint32_t minWidth, uint32_t minHeight) {
	WindowAndThreadStruct wts;
	wts.processId = processId;
	wts.windowHandle = nullptr;
	wts.minWidth = minWidth;
	wts.minHeight = minHeight;

	if (processId > 0) {
		EnumWindows(EnumWindowsProcMy, (LPARAM)&wts);
	}
	return wts.windowHandle;
}

HWND ProcessWatcher::GetWindowByProcessUntilVisible(uint32_t processId, uint32_t minWidth, uint32_t minHeight) {
	HWND resultWindow = GetWindowByProcess(processId, minWidth, minHeight);
	return resultWindow;
}


bool ProcessWatcher::StartProcessFrom(const wchar_t *path, const wchar_t* exeName) {
	std::wstring realPath = std::wstring(path) + std::wstring(exeName);
 	return StartProcessFrom(realPath.c_str());
}

bool ProcessWatcher::StartProcessFrom(const wchar_t *totalPath) {

	STARTUPINFO si = { 0x00 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0x00 };

	wchar_t test[0x200] = { 0x00 };
	swprintf_s(test, L"%s", totalPath);
	bool success = false;
	if (CreateProcessW(nullptr, test, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
		success = true;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return success;
}

void ProcessWatcher::KillProcess(Process& p) {
	if (p.getProcessId() <= 0) {
		return;
	}
	char buffer[0x50] = { 0x00 };
	sprintf_s(buffer, "taskkill /pid %i", p.getProcessId());
	system(buffer);
	//const auto handleOfProcessToKill = OpenProcess(PROCESS_TERMINATE, false, p.getProcessId());
	//int result = TerminateProcess(handleOfProcessToKill, 1);
	//CloseHandle(handleOfProcessToKill);
}