#pragma once

#include "FECoreIncludes.h"

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

		bool checkFile(const char* path);
		std::string getFileExtension(const char* path);
		bool isFolder(const char* path);
		bool createFolder(const char* path);
		bool deleteFolder(const char* path);
		std::vector<std::string> getFolderList(const char* path);
		std::vector<std::string> getFileList(const char* path);
		bool changeFileName(const char* path, const char* newPath);
		bool deleteFile(const char* path);

		char* getDirectoryPath(const char* fullPath);
		char* getFileName(const char* fullPath);

#ifdef FE_WIN_32
		void showFileOpenDialog(std::string& path, const COMDLG_FILTERSPEC* filter, int filterCount = 1);
		void showFolderOpenDialog(std::string& path);

		void showFileSaveDialog(std::string& path, const COMDLG_FILTERSPEC* filter, int filterCount = 1);
		std::string getApplicationPath();
#endif

	private:
		SINGLETON_PRIVATE_PART(FEFileSystem)
#ifdef FE_WIN_32
		std::string PWSTRtoString(PWSTR wString);
#endif
	};

	#define FILE_SYSTEM FEFileSystem::getInstance()
}
