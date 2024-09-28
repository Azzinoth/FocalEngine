#include "FEFileSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetFileSystem()
{
	return FEFileSystem::GetInstancePointer();
}
#endif

FEFileSystem::FEFileSystem() {}
FEFileSystem::~FEFileSystem() {}

bool FEFileSystem::DoesFileExist(const std::string& Path)
{
	std::filesystem::path FilePath(Path);
	return std::filesystem::exists(FilePath) && std::filesystem::is_regular_file(FilePath);
}

bool FEFileSystem::RenameFile(const std::string& Path, const std::string& NewPath)
{
	try
	{
		std::filesystem::rename(Path, NewPath);
		return true;
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::ChangeFileName: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}
}

bool FEFileSystem::CopyFile(const std::string& Path, const std::string& NewPath)
{
	try
	{
		std::filesystem::copy(Path, NewPath);
		return true;
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::CopyFile: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}
}

bool FEFileSystem::DeleteFile(const std::string& Path)
{
	try
	{
		return std::filesystem::remove(Path);
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::DeleteFile: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}
}

size_t FEFileSystem::GetFileSize(const std::string& Path)
{
	std::filesystem::path FilePath(Path);
	if (std::filesystem::exists(FilePath) && std::filesystem::is_regular_file(FilePath))
	{
		return std::filesystem::file_size(FilePath);
	}

	return 0;
}

uint64_t FEFileSystem::GetFileLastWriteTime(const std::string& Path)
{
	std::filesystem::path FilePath(Path);
	if (std::filesystem::exists(FilePath) && std::filesystem::is_regular_file(FilePath))
	{
		auto FileTime = std::filesystem::last_write_time(FilePath);
		auto TimePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(FileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
		auto Duration = TimePoint.time_since_epoch();
		auto NanosecondsCount = std::chrono::duration_cast<std::chrono::nanoseconds>(Duration);
		return NanosecondsCount.count();
	}

	return 0;
}

//void FEFileSystem::WaitForFileAccess(std::string FilePath, int TimeoutInMS)
//{
//	std::filesystem::path Path(FilePath);
//	std::chrono::milliseconds Timeout(TimeoutInMS);
//	std::chrono::time_point<std::chrono::system_clock> StartTime = std::chrono::system_clock::now();
//	std::filesystem::file_status Status = std::filesystem::status(Path);
//	while (std::filesystem::exists(Path) && std::filesystem::is_regular_file(Path))
//	{
//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
//		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - StartTime) > Timeout)
//			break;
//	}
//}

bool FEFileSystem::WaitForFileAccess(const std::string& FilePath, int TimeoutInMS)
{
	const auto start = std::chrono::steady_clock::now();
	const auto end = start + std::chrono::milliseconds(TimeoutInMS);

	while (std::chrono::steady_clock::now() < end)
	{
		try
		{
			// Check if file exists
			if (!std::filesystem::exists(FilePath))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			// Try to open the file
			std::ifstream file(FilePath);
			if (file.is_open())
			{
				// File is accessible, close it and return
				file.close();
				return true;
			}
		}
		catch (const std::filesystem::filesystem_error&)
		{
			// Ignore filesystem errors and continue trying
		}
		catch (const std::ios_base::failure&)
		{
			// Ignore file open errors and continue trying
		}

		// Wait for 10ms before next attempt
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// If we've reached here, we've timed out
	return false;
}

bool FEFileSystem::DoesDirectoryExist(const std::string& Path)
{
	std::filesystem::path FolderPath(Path);
	return std::filesystem::exists(FolderPath) && std::filesystem::is_directory(FolderPath);
}

bool FEFileSystem::RenameDirectory(const std::string& Path, const std::string& NewPath)
{
	try
	{
		if (std::filesystem::exists(Path) && std::filesystem::is_directory(Path))
		{
			std::filesystem::rename(Path, NewPath);
			return true;
		}
		else
		{
			// Directory doesn't exist or is not a directory
			return false;
		}
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::RenameDirectory: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}
}

bool FEFileSystem::CreateDirectory(const std::string& Path)
{
	try
	{
		return std::filesystem::create_directory(Path);
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::CreateDirectory: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}
}

bool FEFileSystem::DeleteDirectory(const std::string& Path)
{
	try
	{
		return std::filesystem::remove_all(Path) > 0;
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::DeleteDirectory: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}
}

std::vector<std::string> FEFileSystem::GetFileNamesInDirectory(const std::string& Path, bool bRecursive)
{
	std::vector<std::string> Result;

	try
	{
		std::filesystem::path Directory(Path);
		if (std::filesystem::exists(Directory) && std::filesystem::is_directory(Directory))
		{
			for (const auto& Entry : std::filesystem::directory_iterator(Directory))
			{
				const auto& Path = Entry.path();
				if (std::filesystem::is_regular_file(Path))
				{
					Result.push_back(Path.filename().string());
				}
				else if (bRecursive && std::filesystem::is_directory(Path))
				{
					std::vector<std::string> SubDirectoryFiles = GetFileNamesInDirectory(Path.string(), bRecursive);
					Result.insert(Result.end(), SubDirectoryFiles.begin(), SubDirectoryFiles.end());
				}
			}
		}
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::GetFileNamesInDirectory: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
	}

	return Result;
}

std::vector<std::string> FEFileSystem::GetFilesInDirectory(const std::string& Path, bool bRecursive)
{
	std::vector<std::string> Result;

	try
	{
		std::filesystem::path Directory(Path);
		if (std::filesystem::exists(Directory) && std::filesystem::is_directory(Directory))
		{
			for (const auto& Entry : std::filesystem::directory_iterator(Directory))
			{
				const auto& Path = Entry.path();
				if (std::filesystem::is_regular_file(Path))
				{
					Result.push_back(Path.string());
				}
				else if (bRecursive && std::filesystem::is_directory(Path))
				{
					std::vector<std::string> SubDirectoryFiles = GetFilesInDirectory(Path.string(), bRecursive);
					Result.insert(Result.end(), SubDirectoryFiles.begin(), SubDirectoryFiles.end());
				}
			}
		}
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::GetFilesInDirectory: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
	}

	return Result;
}

std::vector<std::string> FEFileSystem::GetFolderChain(const std::string& Path)
{
	std::vector<std::string> Result;

	try
	{
		std::filesystem::path Directory(Path);
		while (!Directory.string().empty())
		{
			//Directory.
			//std::filesystem::path test = std::filesystem::canonical(Directory.string() + "/");
			//if (std::filesystem::is_directory(Directory.string() + "/"))

			if (!Result.empty())
			{
				if (Result.back() == Directory.string())
					break;
			}
			Result.push_back(Directory.string());
			Directory = Directory.parent_path();
		}

		std::reverse(Result.begin(), Result.end());
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::GetFolderChain: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
	}

	return Result;
}

std::vector<std::string> FEFileSystem::GetDirectoryList(const std::string& Path)
{
	std::vector<std::string> Result;

	try
	{
		std::filesystem::path Directory(Path);
		if (std::filesystem::exists(Directory) && std::filesystem::is_directory(Directory))
		{
			for (const auto& Entry : std::filesystem::directory_iterator(Directory))
			{
				const auto& Path = Entry.path();
				if (std::filesystem::is_directory(Path))
				{
					std::string DirectoryName = Path.filename().string();
					if (DirectoryName != "." && DirectoryName != "..")
					{
						Result.push_back(DirectoryName);
					}
				}
			}
		}
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::GetFolderList: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
	}

	return Result;
}

std::vector<std::string> FEFileSystem::GetFileList(const std::string& Path)
{
	std::vector<std::string> Result;

	try
	{
		std::filesystem::path Directory(Path);
		if (std::filesystem::exists(Directory) && std::filesystem::is_directory(Directory))
		{
			for (const auto& Entry : std::filesystem::directory_iterator(Directory))
			{
				const auto& Path = Entry.path();
				if (std::filesystem::is_regular_file(Path))
				{
					Result.push_back(Path.filename().string());
				}
			}
		}
	}
	catch (const std::exception& Exception)
	{
		LOG.Add("Error in FEFileSystem::GetFileList: " + std::string(Exception.what()), "FE_FILE_SYSTEM", FE_LOG_ERROR);
	}

	return Result;
}

#ifdef FE_WIN_32
// open dialog staff
std::string FEFileSystem::PWSTRtoString(const PWSTR WString)
{
	const std::wstring WFileName = WString;
	char* SzTo = new char[WFileName.length() + 1];
	SzTo[WFileName.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, WFileName.c_str(), -1, SzTo, static_cast<int>(WFileName.length()), nullptr, nullptr);
	std::string Result = SzTo;
	delete[] SzTo;

	return Result;
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

void FEFileSystem::ShowFileSaveDialog(std::string& Path, const COMDLG_FILTERSPEC* Filter, const int FilterCount, int* ChosenFilterIndex)
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

				// Retrieve the index of the selected filter
				if (ChosenFilterIndex != nullptr)
				{
					unsigned int TempChosenFilterIndex;
					hr = PFileSave->GetFileTypeIndex(&TempChosenFilterIndex);
					*ChosenFilterIndex = TempChosenFilterIndex - 1;

					if (FAILED(hr))
						*ChosenFilterIndex = -1;
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

std::string FEFileSystem::GetExecutablePath()
{
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(nullptr, buffer, MAX_PATH);
	return buffer;
}

#endif

std::string FEFileSystem::GetCurrentWorkingPath()
{
	std::filesystem::path ApplicationPath = std::filesystem::current_path();
	return ApplicationPath.string();
}

std::string FEFileSystem::GetFileExtension(const std::string& Path)
{
	std::filesystem::path filePath(Path);
	return filePath.extension().string();
}

std::string FEFileSystem::GetDirectoryPath(const std::string& FullPath)
{
	std::filesystem::path Path(FullPath);
	return Path.parent_path().string();
}

std::string FEFileSystem::GetFileName(const std::string& FullPath)
{
	std::filesystem::path Path(FullPath);
	return Path.filename().string();
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