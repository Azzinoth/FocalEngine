#include "FEFileSystem.h"
using namespace FocalEngine;
FEFileSystem* FEFileSystem::Instance = nullptr;
FEFileSystem::FEFileSystem() {}
FEFileSystem::~FEFileSystem() {}

bool FEFileSystem::CheckFile(const char* Path)
{
	const DWORD DwAttrib = GetFileAttributesA(Path);
	return (DwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(DwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FEFileSystem::IsFolder(const char* Path)
{
	const DWORD DwAttrib = GetFileAttributesA(Path);
	return (DwAttrib != INVALID_FILE_ATTRIBUTES &&
		(DwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FEFileSystem::CreateFolder(const char* Path)
{
	return (_mkdir(Path) != 0);
}

bool FEFileSystem::DeleteFolder(const char* Path)
{
	return (_rmdir(Path) == 0);
}

bool FEFileSystem::ChangeFileName(const char* Path, const char* NewPath)
{
	const int result = rename(Path, NewPath);
	return result == 0 ? true : false;
}

bool FEFileSystem::DeleteFile(const char* Path)
{
	const int result = remove(Path);
	return result == 0 ? true : false;
}

std::vector<std::string> FEFileSystem::GetFolderList(const char* Path)
{
	std::vector<std::string> result;
	std::string pattern(Path);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE HFind;
	if ((HFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (IsFolder((Path + std::string("/") + std::string(data.cFileName)).c_str()) && std::string(data.cFileName) != std::string(".") && std::string(data.cFileName) != std::string(".."))
				result.push_back(data.cFileName);
		} while (FindNextFileA(HFind, &data) != 0);
		FindClose(HFind);
	}

	return result;
}

std::vector<std::string> FEFileSystem::GetFileList(const char* Path)
{
	std::vector<std::string> result;
	std::string pattern(Path);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE HFind;
	if ((HFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!IsFolder((Path + std::string("/") + std::string(data.cFileName)).c_str()) && std::string(data.cFileName) != std::string(".") && std::string(data.cFileName) != std::string(".."))
				result.push_back(data.cFileName);
		} while (FindNextFileA(HFind, &data) != 0);
		FindClose(HFind);
	}

	return result;
}

#ifdef FE_WIN_32
// open dialog staff
std::string FEFileSystem::PWSTRtoString(const PWSTR WString)
{
	const std::wstring WFileName = WString;
	char* SzTo = new char[WFileName.length() + 1];
	SzTo[WFileName.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, WFileName.c_str(), -1, SzTo, static_cast<int>(WFileName.length()), nullptr, nullptr);
	std::string result = SzTo;
	delete[] SzTo;

	return result;
}

void FEFileSystem::ShowFileOpenDialog(std::string& Path, const COMDLG_FILTERSPEC* Filter, const int FilterCount)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* PFileOpen;
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&PFileOpen));

		if (SUCCEEDED(hr))
		{
			hr = PFileOpen->SetFileTypes(FilterCount, Filter);
			// Show the Open dialog box.
			hr = PFileOpen->Show(nullptr);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* PItem;
				hr = PFileOpen->GetResult(&PItem);
				if (SUCCEEDED(hr))
				{
					PWSTR PszFilePath;
					hr = PItem->GetDisplayName(SIGDN_FILESYSPATH, &PszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						Path = PWSTRtoString(PszFilePath);
						CoTaskMemFree(PszFilePath);
					}
					PItem->Release();
				}
			}
			PFileOpen->Release();
		}
		CoUninitialize();
	}
}

void FEFileSystem::ShowFileSaveDialog(std::string& Path, const COMDLG_FILTERSPEC* Filter, const int FilterCount)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog* PFileSave;
		// Create the FileSaveDialog object.
		hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&PFileSave));

		if (SUCCEEDED(hr))
		{
			hr = PFileSave->SetFileTypes(FilterCount, Filter);
			// Show the Save dialog box.
			hr = PFileSave->Show(nullptr);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* PItem;
				hr = PFileSave->GetResult(&PItem);
				if (SUCCEEDED(hr))
				{
					PWSTR PszFilePath;
					hr = PItem->GetDisplayName(SIGDN_FILESYSPATH, &PszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						Path = PWSTRtoString(PszFilePath);
						CoTaskMemFree(PszFilePath);
					}
					PItem->Release();
				}
			}
			PFileSave->Release();
		}
		CoUninitialize();
	}
}

void FEFileSystem::ShowFolderOpenDialog(std::string& Path)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileDialog* PFolderOpen;
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&PFolderOpen));

		if (SUCCEEDED(hr))
		{
			DWORD DwOptions;
			if (SUCCEEDED(PFolderOpen->GetOptions(&DwOptions)))
				PFolderOpen->SetOptions(DwOptions | FOS_PICKFOLDERS);

			hr = PFolderOpen->SetFileTypes(0, nullptr);
			// Show the Open dialog box.
			hr = PFolderOpen->Show(nullptr);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* PItem;
				hr = PFolderOpen->GetResult(&PItem);
				if (SUCCEEDED(hr))
				{
					PWSTR PszFolderPath;
					hr = PItem->GetDisplayName(SIGDN_FILESYSPATH, &PszFolderPath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						Path = PWSTRtoString(PszFolderPath);
						CoTaskMemFree(PszFolderPath);
					}
					PItem->Release();
				}
			}
			PFolderOpen->Release();
		}
		CoUninitialize();
	}
}

std::string FEFileSystem::GetFileExtension(const char* Path)
{
	// Should I use _splitpath_s ?
	if (!CheckFile(Path))
		return "";

	const LPSTR extension = PathFindExtensionA(Path);
	return std::string(extension);
}

std::string FEFileSystem::GetApplicationPath()
{
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(nullptr, buffer, MAX_PATH);
	return buffer;
}

#endif

char* FEFileSystem::GetDirectoryPath(const char* FullPath)
{
	char* result = new char[1024];
	_splitpath_s(FullPath, nullptr, 0, result, 1024, nullptr, 0, nullptr, 0);

	return result;
}

char* FEFileSystem::GetFileName(const char* FullPath)
{
	char* result = new char[1024];
	_splitpath_s(FullPath, nullptr, 0, nullptr, 0, result, 1024, nullptr, 0);

	return result;
}

std::string FEFileSystem::ReadFEString(std::fstream& File)
{
	char* Buffer = new char[4];

	File.read(Buffer, 4);
	const int TempCharSize = *(int*)Buffer;
	char* TempChar = new char[TempCharSize + 1];
	File.read(TempChar, TempCharSize);
	TempChar[TempCharSize] = '\0';

	std::string Result = TempChar;
	delete[] TempChar;
	delete[] Buffer;

	return Result;
}