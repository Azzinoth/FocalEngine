#pragma once

#include "FEFileSystem.h"
#include "../Core/FEObject.h"

namespace FocalEngine
{
	struct FEAssetPackageAssetInfo
	{
		std::string ID;
		std::string Name;
		std::string Type;
		std::string Tag;
		std::string Comment;

		size_t TimeStamp = 0;
		size_t Size = 0;
		// Offset from the end of the header.
		size_t Offset = 0;
	};

	struct FEAssetPackageEntryIntializeData
	{
		std::string ID;
		std::string Name;
		std::string Type;
		std::string Tag;
		std::string Comment;

		bool IsEmpty()
		{
			return ID.empty() && Name.empty() && Type.empty() && Tag.empty() && Comment.empty();
		}
	};

	struct FEAssetPackageHeader
	{
		size_t Size = 0;
		size_t EntriesCount = 0;
		size_t CurrentAssetOffset = 0;
		size_t FormatVersion = 1;
		size_t BuildTimeStamp = 0;

		std::unordered_map<std::string, FEAssetPackageAssetInfo> Entries;
	};

	class FOCAL_ENGINE_API FEAssetPackage : public FEObject
	{
		friend class FEResourceManager;
	public:
		FEAssetPackage();
		FEAssetPackage(std::string PackageName, std::vector<std::string> FilesToAdd);
		~FEAssetPackage();

		bool LoadFromFile(const std::string& FilePath);
		bool LoadFromMemory(unsigned char* RawData, size_t Size);

		bool SaveToFile(const std::string& FilePath);
		unsigned char* ExportAsRawData(size_t& Size);

		std::string ImportAssetFromFile(const std::string& FilePath, FEAssetPackageEntryIntializeData IntializeData = FEAssetPackageEntryIntializeData());
		bool ExportAssetToFile(const std::string& ID, const std::string& FilePath);
		bool IsAssetIDPresent(const std::string& ID);

		FEAssetPackageAssetInfo GetAssetInfo(const std::string& ID);
		std::vector<FEAssetPackageAssetInfo> GetEntryList();
		char* GetAssetDataCopy(const std::string& ID);
		
		size_t GetBuildTimeStamp();
		std::string GetBuildTimeStampAsString();
	private:
		// String that each asset package has to start with.
		static std::string HeaderStartPhrase;

		FEAssetPackageHeader Header;

		void UpdateHeaderSize();

		//void WriteHeader(std::ofstream& OpenedFile);
		//bool ReadHeader(std::ifstream& OpenedFile);

		// In memory representation of the asset package.
		std::vector<char> Data;
	};
}
