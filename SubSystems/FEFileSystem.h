#pragma once

#include "FECoreIncludes.h"

#ifdef FE_WIN_32
	#include <direct.h> // file system
	#include <shobjidl.h> // openDialog
#endif

namespace FocalEngine
{
	class FEFileSystem
	{
	public:
		SINGLETON_PUBLIC_PART(FEFileSystem)

		bool checkFolder(const char* dirPath);
		bool createFolder(const char* dirPath);
		bool deleteFolder(const char* dirPath);
		std::vector<std::string> getFolderList(const char* dirPath);
		bool changeFileName(const char* filePath, const char* newName);
		bool deleteFile(const char* filePath);

#ifdef FE_WIN_32
		void openDialog(std::string& filePath, const COMDLG_FILTERSPEC* filter, int filterCount = 1);
#endif

	private:
		SINGLETON_PRIVATE_PART(FEFileSystem)
#ifdef FE_WIN_32
		std::string PWSTRtoString(PWSTR wString);
#endif
	};

	#define FILESYSTEM FEFileSystem::getInstance()
}
