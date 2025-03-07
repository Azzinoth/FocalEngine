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

		bool DoesFileExist(const std::string& Path);
		bool RenameFile(const std::string& Path, const std::string& NewPath);
		bool CopyFile(const std::string& Path, const std::string& NewPath);
		bool DeleteFile(const std::string& Path);
		std::vector<std::string> GetFileList(const std::string& Path);
		size_t GetFileSize(const std::string& Path);
		// Returns last write time in nanoseconds since epoch.
		uint64_t GetFileLastWriteTime(const std::string& Path);
		bool WaitForFileAccess(const std::string& FilePath, int TimeoutInMS = 1000);

		std::string GetFileExtension(const std::string& Path);
		std::string GetDirectoryPath(const std::string& FullPath);
		std::string GetFileName(const std::string& FullPath);

		bool DoesDirectoryExist(const std::string& Path);
		bool RenameDirectory(const std::string& Path, const std::string& NewPath);
		bool CreateDirectory(const std::string& Path);
		bool CopyDirectory(const std::string& Path, const std::string& NewPath);
		bool DeleteDirectory(const std::string& Path);

		std::vector<std::string> GetFilesInDirectory(const std::string& Path, bool bRecursive = false);
		std::vector<std::string> GetFileNamesInDirectory(const std::string& Path, bool bRecursive = false);
		std::vector<std::string> GetDirectoryList(const std::string& Path);

		std::vector<std::string> GetFolderChain(const std::string& Path);

		struct TextReplacementRule
		{
			// Pattern to identify the line where replacement should occur
			std::string ContextPattern;
			// Text to be replaced
			std::string TargetText;
			std::string ReplacementText;
		};

		bool PerformTextReplacements(const std::string& FilePath, const std::vector<TextReplacementRule>& Rules);

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

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetFileSystem();
	#define FILE_SYSTEM (*static_cast<FEFileSystem*>(GetFileSystem()))
#else
	#define FILE_SYSTEM FEFileSystem::GetInstance()
#endif
}
