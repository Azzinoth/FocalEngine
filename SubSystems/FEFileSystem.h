#pragma once

#include "../Core/FECoreIncludes.h"
#include <filesystem>

#ifdef FE_WIN_32
	#include <direct.h> // file system
	#include <shobjidl.h> // openDialog
	#include <shlwapi.h> // PathFindExtensionA
	#pragma comment(lib, "shlwapi.lib") // PathFindExtensionA
#endif

namespace FocalEngine
{
	class FEFileSystem
	{
	public:
		SINGLETON_PUBLIC_PART(FEFileSystem)

		bool CheckFile(const char* Path);
		std::string GetFileExtension(const char* Path);
		bool IsFolder(const char* Path);
		bool CreateFolder(const char* Path);
		bool DeleteFolder(const char* Path);
		std::vector<std::string> GetFolderList(const char* Path);
		std::vector<std::string> GetFileList(const char* Path);
		std::vector<std::string> GetFileList(const std::string& Path);
		bool ChangeFileName(const char* Path, const char* NewPath);
		bool DeleteFile(const char* Path);

		char* GetDirectoryPath(const char* FullPath);
		std::string GetDirectoryPath(const std::string& FullPath);
		char* GetFileName(const char* FullPath);
		std::string GetFileName(const std::string& FullPath);

#ifdef FE_WIN_32
		void ShowFileOpenDialog(std::string& Path, const COMDLG_FILTERSPEC* Filter, int FilterCount = 1);
		void ShowFolderOpenDialog(std::string& Path);

		void ShowFileSaveDialog(std::string& Path, const COMDLG_FILTERSPEC* Filter, int FilterCount = 1, int* ChosenFilterIndex = nullptr);
		std::string GetExecutablePath();
#endif
		std::string GetCurrentWorkingPath();
		std::string ReadFEString(std::fstream& File);
	private:
		SINGLETON_PRIVATE_PART(FEFileSystem)
#ifdef FE_WIN_32
		std::string PWSTRtoString(PWSTR WString);
#endif
	};

	#define FILE_SYSTEM FEFileSystem::getInstance()
}
