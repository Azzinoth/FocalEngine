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

		bool checkFile(const char* path);
		bool checkFolder(const char* path);
		bool createFolder(const char* path);
		bool deleteFolder(const char* path);
		std::vector<std::string> getFolderList(const char* path);
		bool changeFileName(const char* path, const char* newPath);
		bool deleteFile(const char* path);

#ifdef FE_WIN_32
		void openDialog(std::string& path, const COMDLG_FILTERSPEC* filter, int filterCount = 1);
#endif

	private:
		SINGLETON_PRIVATE_PART(FEFileSystem)
#ifdef FE_WIN_32
		std::string PWSTRtoString(PWSTR wString);
#endif
	};

	#define FILESYSTEM FEFileSystem::getInstance()
}
