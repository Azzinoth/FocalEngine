#pragma once

#include "../../Core/FECoreIncludes.h"

namespace FocalEngine
{
	struct FEZipArchiveFile
	{
		std::string Name;
		std::vector<unsigned char> Data;
	};

	class FOCAL_ENGINE_API FEZipArchive
	{
	public:
		static bool ReadFiles(const std::string& FilePath, std::vector<FEZipArchiveFile>& OutFiles, std::string& OutErrorMessage, const std::function<bool(const std::string& FileName)>& ShouldReadFile = nullptr);
	};
}