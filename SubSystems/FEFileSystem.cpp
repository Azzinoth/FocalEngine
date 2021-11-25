#include "FEFileSystem.h"
using namespace FocalEngine;
FEFileSystem* FEFileSystem::_instance = nullptr;
FEFileSystem::FEFileSystem() {}
FEFileSystem::~FEFileSystem() {}

bool FEFileSystem::checkFile(const char* path)
{
	DWORD dwAttrib = GetFileAttributesA(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FEFileSystem::isFolder(const char* path)
{
	DWORD dwAttrib = GetFileAttributesA(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FEFileSystem::createFolder(const char* path)
{
	return (_mkdir(path) != 0);
}

bool FEFileSystem::deleteFolder(const char* path)
{
	return (_rmdir(path) == 0);
}

bool FEFileSystem::changeFileName(const char* path, const char* newPath)
{
	int result = rename(path, newPath);
	return result == 0 ? true : false;
}

bool FEFileSystem::deleteFile(const char* path)
{
	int result = remove(path);
	return result == 0 ? true : false;
}

std::vector<std::string> FEFileSystem::getFolderList(const char* path)
{
	std::vector<std::string> result;
	std::string pattern(path);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (isFolder((path + std::string("/") + std::string(data.cFileName)).c_str()) && std::string(data.cFileName) != std::string(".") && std::string(data.cFileName) != std::string(".."))
				result.push_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}

	return result;
}

std::vector<std::string> FEFileSystem::getFileList(const char* path)
{
	std::vector<std::string> result;
	std::string pattern(path);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!isFolder((path + std::string("/") + std::string(data.cFileName)).c_str()) && std::string(data.cFileName) != std::string(".") && std::string(data.cFileName) != std::string(".."))
				result.push_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}

	return result;
}

#ifdef FE_WIN_32
// open dialog staff
std::string FEFileSystem::PWSTRtoString(PWSTR wString)
{
	std::wstring wFileName = wString;
	char* szTo = new char[wFileName.length() + 1];
	szTo[wFileName.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, wFileName.c_str(), -1, szTo, (int)wFileName.length(), NULL, NULL);
	std::string result = szTo;
	delete[] szTo;

	return result;
}

void FEFileSystem::showFileOpenDialog(std::string& path, const COMDLG_FILTERSPEC* filter, int filterCount)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			hr = pFileOpen->SetFileTypes(filterCount, filter);
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						path = PWSTRtoString(pszFilePath);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
}

void FEFileSystem::showFileSaveDialog(std::string& path, const COMDLG_FILTERSPEC* filter, int filterCount)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog* pFileSave;
		// Create the FileSaveDialog object.
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

		if (SUCCEEDED(hr))
		{
			hr = pFileSave->SetFileTypes(filterCount, filter);
			// Show the Save dialog box.
			hr = pFileSave->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileSave->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						path = PWSTRtoString(pszFilePath);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileSave->Release();
		}
		CoUninitialize();
	}
}

void FEFileSystem::showFolderOpenDialog(std::string& path)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileDialog* pFolderOpen;
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFolderOpen));

		if (SUCCEEDED(hr))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pFolderOpen->GetOptions(&dwOptions)))
				pFolderOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);

			hr = pFolderOpen->SetFileTypes(0, nullptr);
			// Show the Open dialog box.
			hr = pFolderOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFolderOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFolderPath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						path = PWSTRtoString(pszFolderPath);
						CoTaskMemFree(pszFolderPath);
					}
					pItem->Release();
				}
			}
			pFolderOpen->Release();
		}
		CoUninitialize();
	}
}

std::string FEFileSystem::getFileExtension(const char* path)
{
	// Should I use _splitpath_s ?
	if (!checkFile(path))
		return "";

	LPSTR extension = PathFindExtensionA(path);
	return std::string(extension);
}

std::string FEFileSystem::getApplicationPath()
{
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return buffer;
}

#endif

char* FEFileSystem::getDirectoryPath(const char* fullPath)
{
	char* result = new char[1024];
	_splitpath_s(fullPath, nullptr, 0, result, 1024, nullptr, 0, nullptr, 0);

	return result;
}

char* FEFileSystem::getFileName(const char* fullPath)
{
	char* result = new char[1024];
	_splitpath_s(fullPath, nullptr, 0, nullptr, 0, result, 1024, nullptr, 0);

	return result;
}