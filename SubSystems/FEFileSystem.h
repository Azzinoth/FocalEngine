#pragma once

#include "../Core/FECoreIncludes.h"
#include <filesystem>

#ifdef FE_WIN_32
	#include <shobjidl.h> // OpenDialog
#endif

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEFileSystem
	{
	public:
		SINGLETON_PUBLIC_PART(FEFileSystem)

		bool CheckFile(const std::string& Path);
		bool RenameFile(const std::string& Path, const std::string& NewPath);
		bool DeleteFile(const std::string& Path);
		std::vector<std::string> GetFileList(const std::string& Path);

		std::string GetFileExtension(const std::string& Path);
		std::string GetDirectoryPath(const std::string& FullPath);
		std::string GetFileName(const std::string& FullPath);

		bool CheckDirectory(const std::string& Path);
		bool RenameDirectory(const std::string& Path, const std::string& NewPath);
		bool CreateDirectory(const std::string& Path);
		bool DeleteDirectory(const std::string& Path);
		std::vector<std::string> GetDirectoryList(const std::string& Path);

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

	#define FILE_SYSTEM FEFileSystem::GetInstance()
}
