#ifndef __LEAGUE_FILE_DIALOG__
#define __LEAGUE_FILE_DIALOG__


#include <Windows.h>
#include <string.h>
#include <iostream>
#include <shobjidl.h> 
#include <objbase.h>      // For COM headers
#include <shlwapi.h>

enum class FileDialogResultType : uint8_t {
	SUCCESS,
	ABORTED,
	FAILED
};

class FileDialogResult {
private:
	FileDialogResultType resultType;
	std::wstring filePath;
	friend class FileDialog;
public:
	FileDialogResult() : FileDialogResult(FileDialogResultType::FAILED, L"") {}
	FileDialogResult(FileDialogResultType type, std::wstring filePath) {
		this->resultType = type;
		this->filePath = filePath;
	}
	virtual ~FileDialogResult() {
		
	}
	FileDialogResult& operator=(const FileDialogResult& other) = default;
	inline std::wstring getFilePath() const {
		return filePath;
	}
	inline FileDialogResultType getResultType() const {
		return resultType;
	}
};

class FileDialog {
public:
	FileDialog() {}
	virtual ~FileDialog() {}

	FileDialogResult openFileDialogTest() {
		std::wstring fileNameStr = std::wstring(L"");
		IFileOpenDialog *pFileOpen; 

		// Create the FileOpenDialog object.
		FileDialogResult result;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,	IID_PPV_ARGS(&pFileOpen));
		if (SUCCEEDED(hr))
		{
			COMDLG_FILTERSPEC specs = { 0x00 };;
			specs.pszName = L"Images";
			specs.pszSpec = L"*.png;*.jpg;*.jpeg;*.bmp";
			pFileOpen->SetFileTypes(1, &specs);
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			
			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					
					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						result.resultType = FileDialogResultType::SUCCESS;
						result.filePath = std::wstring(pszFilePath);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
				result.resultType = FileDialogResultType::ABORTED;
			}
			pFileOpen->Release();			
		}
		return result;
	}
};

#endif 