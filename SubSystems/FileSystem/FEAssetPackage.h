#pragma once

#include "FEFileSystem.h"
#include "../Core/FEObject.h"

namespace FocalEngine
{
	struct FEAssetPackageAssetInfo
	{
		FEUUID ID;
		std::string Name;
		std::string Type;
		std::string Tag;
		std::string Comment;

		size_t TimeStamp = 0;
		size_t Size = 0;
		// Offset from the end of the header.
		size_t Offset = 0;
	};

	struct FEAssetPackageEntryInitializeData
	{
		FEUUID ID;
		std::string Name;
		std::string Type;
		std::string Tag;
		std::string Comment;

		bool IsEmpty()
		{
			return UNIQUE_ID.IsNull(ID) && Name.empty() && Type.empty() && Tag.empty() && Comment.empty();
		}
	};

	struct FEAssetPackageHeader
	{
		size_t Size = 0;
		size_t EntriesCount = 0;
		size_t CurrentAssetOffset = 0;
		size_t FormatVersion = 1;
		size_t BuildTimeStamp = 0;

		std::unordered_map<FEUUID, FEAssetPackageAssetInfo> Entries;
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

		FEUUID ImportAssetFromFile(const std::string& FilePath, FEAssetPackageEntryInitializeData InitializeData = FEAssetPackageEntryInitializeData());
		bool UpdateAssetFromFile(const FEUUID& ID, const std::string& FilePath);
		bool ExportAssetToFile(const FEUUID& ID, const std::string& FilePath);

		FEUUID ImportAssetFromMemory(unsigned char* RawData, size_t Size, FEAssetPackageEntryInitializeData InitializeData = FEAssetPackageEntryInitializeData());
		bool UpdateAssetFromMemory(const FEUUID& ID, unsigned char* RawData, size_t Size);
		bool ExportAssetToMemory(const FEUUID& ID, unsigned char*& RawData, size_t& Size);

		// Returns the ID of the asset.
		// Accepts a FEObject* to get the asset data from.
		FEUUID ImportAsset(FEObject* Object, FEAssetPackageEntryInitializeData InitializeData = FEAssetPackageEntryInitializeData());

		bool IsAssetIDPresent(const FEUUID& ID);
		bool RemoveAsset(const FEUUID& ID);

		FEAssetPackageAssetInfo GetAssetInfo(const FEUUID& ID);
		std::vector<FEAssetPackageAssetInfo> GetEntryList();
		char* GetAssetDataCopy(const FEUUID& ID);

		size_t GetBuildTimeStamp();
		std::string GetBuildTimeStampAsString();

		std::vector<FEUUID> GetAssetIDsByName(const std::string& Name);
	private:
		// String that each asset package has to start with.
		static std::string HeaderStartPhrase;

		FEAssetPackageHeader Header;

		void UpdateHeaderSize();
		size_t CalculateHeaderSize();
		// In memory representation of the asset package.
		std::vector<char> Data;
	};
}
