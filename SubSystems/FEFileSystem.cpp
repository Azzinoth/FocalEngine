#include "FEFileSystem.h"
using namespace FocalEngine;
FEFileSystem* FEFileSystem::_instance = nullptr;
FEFileSystem::FEFileSystem() {}
FEFileSystem::~FEFileSystem() {}

bool FEFileSystem::checkFolder(const char* dirPath)
{
	DWORD dwAttrib = GetFileAttributesA(dirPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FEFileSystem::createFolder(const char* dirPath)
{
	return (_mkdir(dirPath) != 0);
}

bool FEFileSystem::deleteFolder(const char* dirPath)
{
	return (_rmdir(dirPath) == 0);
}

bool FEFileSystem::changeFileName(const char* filePath, const char* newName)
{
	int result = rename(filePath, newName);
	return result == 0 ? true : false;
}

bool FEFileSystem::deleteFile(const char* filePath)
{
	int result = remove(filePath);
	return result == 0 ? true : false;
}

std::vector<std::string> FEFileSystem::getFolderList(const char* dirPath)
{
	std::vector<std::string> result;
	std::string pattern(dirPath);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (std::string(data.cFileName) != std::string(".") && std::string(data.cFileName) != std::string(".."))
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

void FEFileSystem::openDialog(std::string& filePath, const COMDLG_FILTERSPEC* filter, int filterCount)
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
						filePath = PWSTRtoString(pszFilePath);
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

#endif