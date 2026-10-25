#include "FEAssetPackage.h"
#include "../ResourceManager/FEResourceManager.h"
#include "../SubSystems/Scene/FESceneManager.h"
#include <cctype>
using namespace FocalEngine;

// Converts asset name to relative path with '/' separators.
// Returns empty string if name is empty or could lead outside of directory it is exported to.
static std::string ToSafeRelativePath(const std::string& AssetName)
{
	std::string Result;
	std::string Part;
	auto AddPart = [&]() -> bool
	{
		if (Part.empty() || Part == ".")
		{
			Part.clear();
			return true;
		}

		// ':' is used by drive letters and by alternate data streams.
		if (Part == ".." || Part.find(':') != std::string::npos)
			return false;

		if (!Result.empty())
			Result += '/';

		Result += Part;
		Part.clear();
		return true;
	};

	for (const char Character : AssetName)
	{
		if (Character == '/' || Character == '\\')
		{
			if (!AddPart())
				return "";
		}
		else
		{
			Part += Character;
		}
	}

	if (!AddPart())
		return "";

	return Result;
}

std::string FEAssetPackage::HeaderStartPhrase = "FEAssetPackage";
FEAssetPackage::FEAssetPackage() : FEObject(FE_OBJECT_TYPE::FE_ASSET_PACKAGE, "Unnamed asset package") {}

FEAssetPackage::FEAssetPackage(std::string PackageName, std::vector<std::string> FilesToAdd) : FEObject(FE_OBJECT_TYPE::FE_ASSET_PACKAGE, PackageName)
{
	for (size_t i = 0; i < FilesToAdd.size(); i++)
	{
		if (UNIQUE_ID.IsNull(ImportAssetFromFile(FilesToAdd[i])))
		{
			LOG.Add("FEAssetPackage::FEAssetPackage: Could not add file to the package: " + FilesToAdd[i], "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		}
	}
}

bool FEAssetPackage::IsAssetIDPresent(const FEUUID& ID)
{
	return Header.Entries.find(ID) != Header.Entries.end();
}

FEUUID FEAssetPackage::ImportAssetFromFile(const std::string& FilePath, FEAssetPackageEntryInitializeData InitializeData)
{
	// For time being, we will not support hierarchical asset packages.
	// Because of this, we will not support adding directories to the asset package.
	if (FILE_SYSTEM.DoesDirectoryExist(FilePath))
	{
		LOG.Add("FEAssetPackage::ImportAssetFromFile: Directories are not supported in asset packages: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEUUID();
	}

	// First check if file exists.
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEAssetPackage::ImportAssetFromFile: File does not exist: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEUUID();
	}

	FEUUID IDToUse = UNIQUE_ID.IsNull(InitializeData.ID) ? UNIQUE_ID.GenerateID() : InitializeData.ID;
	if (IsAssetIDPresent(IDToUse))
	{
		LOG.Add("FEAssetPackage::ImportAssetFromFile: Asset ID already present: " + UNIQUE_ID.ToString(IDToUse), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEUUID();
	}

	std::string NameToUse = InitializeData.IsEmpty() ? FILE_SYSTEM.GetFileName(FilePath) : InitializeData.Name.empty() ? FILE_SYSTEM.GetFileName(FilePath) : InitializeData.Name;
	std::string TypeToUse = InitializeData.IsEmpty() ? "" : InitializeData.Type;
	std::string TagToUse = InitializeData.IsEmpty() ? "" : InitializeData.Tag;
	std::string CommentToUse = InitializeData.IsEmpty() ? "" : InitializeData.Comment;

	// Now we will try to read raw data from the file.
	std::ifstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("FEAssetPackage::ImportAssetFromFile: Could not open file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEUUID();
	}

	File.seekg(0, std::ios::end);
	size_t FileSize = File.tellg();
	File.seekg(0, std::ios::beg);

	std::vector<char> FileData(FileSize);
	File.read(FileData.data(), FileSize);
	File.close();

	// Now we will add the file to the asset package data.
	FEAssetPackageAssetInfo NewEntry;
	NewEntry.ID = IDToUse;
	NewEntry.Name = NameToUse;
	NewEntry.TimeStamp = FILE_SYSTEM.GetFileLastWriteTime(FilePath);
	NewEntry.Size = FileSize;
	NewEntry.Offset = Data.size();
	NewEntry.Type = TypeToUse;
	NewEntry.Tag = TagToUse;
	NewEntry.Comment = CommentToUse;

	Header.Entries[NewEntry.ID] = NewEntry;

	Header.EntriesCount++;
	UpdateHeaderSize();

	// And finally, add the file data to the asset package data.
	Data.insert(Data.end(), FileData.begin(), FileData.end());

	return IDToUse;
}

FEUUID FEAssetPackage::ImportAssetFromMemory(unsigned char* RawData, size_t Size, FEAssetPackageEntryInitializeData InitializeData)
{
	FEUUID IDToUse = UNIQUE_ID.IsNull(InitializeData.ID) ? UNIQUE_ID.GenerateID() : InitializeData.ID;
	if (IsAssetIDPresent(IDToUse))
	{
		LOG.Add("FEAssetPackage::ImportAssetFromMemory: Asset ID already present: " + UNIQUE_ID.ToString(IDToUse), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEUUID();
	}

	std::string NameToUse = InitializeData.IsEmpty() ? "" : InitializeData.Name;
	std::string TypeToUse = InitializeData.IsEmpty() ? "" : InitializeData.Type;
	std::string TagToUse = InitializeData.IsEmpty() ? "" : InitializeData.Tag;
	std::string CommentToUse = InitializeData.IsEmpty() ? "" : InitializeData.Comment;

	// Now we will add object to the asset package data.
	FEAssetPackageAssetInfo NewEntry;
	NewEntry.ID = IDToUse;
	NewEntry.Name = NameToUse;
	NewEntry.TimeStamp = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	NewEntry.Size = Size;
	NewEntry.Offset = Data.size();
	NewEntry.Type = TypeToUse;
	NewEntry.Tag = TagToUse;
	NewEntry.Comment = CommentToUse;

	Header.Entries[NewEntry.ID] = NewEntry;

	Header.EntriesCount++;
	UpdateHeaderSize();

	// And finally, add the object data to the asset package data.
	Data.insert(Data.end(), RawData, RawData + Size);

	return IDToUse;
}

FEUUID FEAssetPackage::ImportAsset(FEObject* Object, FEAssetPackageEntryInitializeData InitializeData)
{
	if (Object == nullptr)
	{
		LOG.Add("FEAssetPackage::ImportAsset: Object is nullptr.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEUUID();
	}

	FEUUID IDToUse = UNIQUE_ID.IsNull(InitializeData.ID) ? UNIQUE_ID.GenerateID() : InitializeData.ID;
	if (IsAssetIDPresent(IDToUse))
	{
		LOG.Add("FEAssetPackage::ImportAsset: Asset ID already present: " + UNIQUE_ID.ToString(IDToUse), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEUUID();
	}

	std::string NameToUse = InitializeData.IsEmpty() ? Object->GetName() : InitializeData.Name.empty() ? Object->GetName() : InitializeData.Name;
	std::string TypeToUse = InitializeData.IsEmpty() ? "" : InitializeData.Type;
	std::string TagToUse = InitializeData.IsEmpty() ? Object->GetTag() : InitializeData.Tag;
	std::string CommentToUse = InitializeData.IsEmpty() ? "" : InitializeData.Comment;

	// FE_TO_DO: This should save info to memory, not to file.
	switch (Object->GetType())
	{
		case FE_OBJECT_TYPE::FE_TEXTURE:
		{
			FETexture* Texture = reinterpret_cast<FETexture*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_TEXTURE";

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempTexture.texture";
			RESOURCE_MANAGER.SaveFETexture(Texture, FilePath.c_str());
			FEUUID ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.RemoveFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_MESH:
		{
			FEMesh* Mesh = reinterpret_cast<FEMesh*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_MESH";

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempMesh.mesh";
			RESOURCE_MANAGER.SaveFEMesh(Mesh, FilePath.c_str());
			FEUUID ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.RemoveFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_MATERIAL:
		{
			FEMaterial* Material = reinterpret_cast<FEMaterial*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_MATERIAL";

			Json::Value JsonRepresentation = RESOURCE_MANAGER.SaveMaterialToJSON(Material);

			Json::StreamWriterBuilder Builder;
			const std::string JsonFile = Json::writeString(Builder, JsonRepresentation);

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempMaterial.material";
			std::ofstream ResourcesFile;
			ResourcesFile.open(FilePath);
			ResourcesFile << JsonFile;
			ResourcesFile.close();

			FEUUID ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.RemoveFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_GAMEMODEL:
		{
			FEGameModel* GameModel = reinterpret_cast<FEGameModel*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_GAMEMODEL";

			Json::Value JsonRepresentation = RESOURCE_MANAGER.SaveGameModelToJSON(GameModel);

			Json::StreamWriterBuilder Builder;
			const std::string JsonFile = Json::writeString(Builder, JsonRepresentation);

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempGameModel.gamemodel";
			std::ofstream ResourcesFile;
			ResourcesFile.open(FilePath);
			ResourcesFile << JsonFile;
			ResourcesFile.close();

			FEUUID ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.RemoveFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_PREFAB:
		{
			FEPrefab* Prefab = reinterpret_cast<FEPrefab*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_PREFAB";

			Json::Value JsonRepresentation = RESOURCE_MANAGER.SavePrefabToJSON(Prefab);

			Json::StreamWriterBuilder Builder;
			const std::string JsonFile = Json::writeString(Builder, JsonRepresentation);

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempPrefab.prefab";
			std::ofstream ResourcesFile;
			ResourcesFile.open(FilePath);
			ResourcesFile << JsonFile;
			ResourcesFile.close();

			FEUUID ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.RemoveFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_NATIVE_SCRIPT_MODULE:
		{
			FENativeScriptModule* NativeScriptModule = reinterpret_cast<FENativeScriptModule*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_NATIVE_SCRIPT_MODULE";

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempNativeScriptModule.nativescriptmodule";
			RESOURCE_MANAGER.SaveFENativeScriptModule(NativeScriptModule, FilePath);
			FEUUID ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.RemoveFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_SCENE:
		{
			FEScene* Scene = reinterpret_cast<FEScene*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_SCENE";

			Json::Value JsonRepresentation = SCENE_MANAGER.SaveSceneToJSON(Scene);
			Json::StreamWriterBuilder Builder;
			const std::string JsonFile = Json::writeString(Builder, JsonRepresentation);

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempScene.scene";
			std::ofstream ResourcesFile;
			ResourcesFile.open(FilePath);
			ResourcesFile << JsonFile;
			ResourcesFile.close();

			FEUUID ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.RemoveFile(FilePath);

			return ResultingID;
		}

		default:
		{
			LOG.Add("FEAssetPackage::ImportAsset: Object type not supported: " + Object->GetName(), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
			return FEUUID();
		}
	}

	return FEUUID();
}

bool FEAssetPackage::RemoveAsset(const FEUUID& ID)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::RemoveAsset: Asset ID not present: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackageAssetInfo EntryToDelete = Header.Entries[ID];
	Data.erase(Data.begin() + EntryToDelete.Offset, Data.begin() + EntryToDelete.Offset + EntryToDelete.Size);

	Header.Entries.erase(ID);
	Header.EntriesCount--;
	UpdateHeaderSize();

	// Now we need to update the offsets of the assets that are after the removed asset.
	for (auto& Entry : Header.Entries)
	{
		if (Entry.second.Offset > EntryToDelete.Offset)
			Entry.second.Offset -= EntryToDelete.Size;
	}

	return true;
}

bool FEAssetPackage::UpdateAssetFromFile(const FEUUID& ID, const std::string& FilePath)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromFile: Asset ID not present: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromFile: File does not exist: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackageAssetInfo OldEntryInfo = Header.Entries[ID];
	if (!RemoveAsset(ID))
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromFile: Could not remove old asset: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEUUID NewID = ImportAssetFromFile(FilePath, FEAssetPackageEntryInitializeData{ OldEntryInfo.ID, OldEntryInfo.Name, OldEntryInfo.Type, OldEntryInfo.Tag, OldEntryInfo.Comment });
	if (UNIQUE_ID.IsNull(NewID))
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromFile: Could not import file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	if (NewID != ID)
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromFile: New ID is different from the old ID.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEAssetPackage::UpdateAssetFromMemory(const FEUUID& ID, unsigned char* RawData, size_t Size)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromMemory: Asset ID not present: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackageAssetInfo OldEntryInfo = Header.Entries[ID];
	if (!RemoveAsset(ID))
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromMemory: Could not remove old asset: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEUUID NewID = ImportAssetFromMemory(RawData, Size, FEAssetPackageEntryInitializeData{ OldEntryInfo.ID, OldEntryInfo.Name, OldEntryInfo.Type, OldEntryInfo.Tag, OldEntryInfo.Comment });
	if (UNIQUE_ID.IsNull(NewID))
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromMemory: Could not import file from memory.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	if (NewID != ID)
	{
		LOG.Add("FEAssetPackage::UpdateAssetFromMemory: New ID is different from the old ID.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

void FEAssetPackage::UpdateHeaderSize()
{
	// Update header time stamp.
	Header.BuildTimeStamp = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);
	Header.Size = CalculateHeaderSize();
}

size_t FEAssetPackage::CalculateHeaderSize()
{
	size_t Result = HeaderStartPhrase.size();
	// Size of variable that holds the header size.
	Result += sizeof(size_t);
	// Size of variable that holds the format version.
	Result += sizeof(size_t);
	// Size of variable that holds the build time stamp.
	Result += sizeof(size_t);
	// Size of variable that holds the number of assets.
	Result += sizeof(size_t);

	// Now will go through all the asset package entries and add their sizes.
	for (auto& Entry : Header.Entries)
	{
		Result += sizeof(size_t);
		Result += UNIQUE_ID.ToString(Entry.second.ID).size();
		Result += sizeof(size_t);
		Result += Entry.second.Name.size();
		Result += sizeof(size_t);
		Result += Entry.second.Type.size();
		Result += sizeof(size_t);
		Result += Entry.second.Tag.size();
		Result += sizeof(size_t);
		Result += Entry.second.Comment.size();

		// Size of variable that holds the size, offset of asset and time stamp.
		Result += sizeof(size_t) * 3;
	}

	return Result;
}

FEAssetPackage::~FEAssetPackage() {}

bool FEAssetPackage::SaveToFile(const std::string& FilePath)
{
	std::ofstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("FEAssetPackage::SaveToFile: Could not open file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	// Here we will use ExportAsRawData to get the raw data of the asset package.
	size_t DataSize = 0;
	unsigned char* RawData = ExportAsRawData(DataSize);
	if (RawData == nullptr)
	{
		LOG.Add("FEAssetPackage::SaveToFile: Could not export asset package as raw data.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		File.close();
		FILE_SYSTEM.RemoveFile(FilePath);
		return false;
	}

	if (DataSize == 0)
	{
		LOG.Add("FEAssetPackage::SaveToFile: Asset package size is 0.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		File.close();
		delete[] RawData;
		FILE_SYSTEM.RemoveFile(FilePath);
		return false;
	}

	File.write((char*)RawData, DataSize);
	delete[] RawData;

	// Sanity check, if file written data is not equal to the header size, log an error.
	size_t WrittenDataSize = File.tellp();
	if (WrittenDataSize != Header.Size + Data.size())
	{
		LOG.Add("FEAssetPackage::SaveToFile: package size is not equal to the written data size.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		File.close();
		FILE_SYSTEM.RemoveFile(FilePath);
		return false;
	}

	return true;
}

bool FEAssetPackage::LoadFromFile(const std::string& FilePath)
{
	std::ifstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("FEAssetPackage::LoadFromFile: Could not open file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	// Here we will use LoadFromMemory with the raw data of the file.
	File.seekg(0, std::ios::end);
	size_t FileSize = File.tellg();
	File.seekg(0, std::ios::beg);

	unsigned char* RawData = new unsigned char[FileSize];
	File.read((char*)RawData, FileSize);
	File.close();

	const bool bLoaded = LoadFromMemory(RawData, FileSize);
	delete[] RawData;

	if (!bLoaded)
	{
		LOG.Add("FEAssetPackage::LoadFromFile: Could not properly load asset package from file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

FEAssetPackageAssetInfo FEAssetPackage::GetAssetInfo(const FEUUID& ID)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::GetAssetInfo: Asset ID not present: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return FEAssetPackageAssetInfo();
	}

	return Header.Entries[ID];
}

std::vector<FEAssetPackageAssetInfo> FEAssetPackage::GetEntryList()
{
	std::vector<FEAssetPackageAssetInfo> Result;
	for (auto& Entry : Header.Entries)
	{
		Result.push_back(Entry.second);
	}

	return Result;
}

char* FEAssetPackage::GetAssetDataCopy(const FEUUID& ID)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::GetAssetDataCopy: Asset ID not present: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return nullptr;
	}

	FEAssetPackageAssetInfo& Entry = Header.Entries[ID];
	char* DataToReturn = new char[Entry.Size];
	memcpy(DataToReturn, Data.data() + Entry.Offset, Entry.Size);

	return DataToReturn;
}

bool FEAssetPackage::ExportAssetToFile(const FEUUID& ID, const std::string& FilePath)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::ExportAssetToFile: Asset ID not present: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackageAssetInfo& Entry = Header.Entries[ID];
	std::ofstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("FEAssetPackage::ExportAssetToFile: Could not open file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	File.write(Data.data() + Entry.Offset, Entry.Size);
	File.close();
	if (File.fail())
	{
		LOG.Add("FEAssetPackage::ExportAssetToFile: Could not write file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEAssetPackage::ImportDirectory(const std::string& DirectoryPath)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(DirectoryPath))
	{
		LOG.Add("FEAssetPackage::ImportDirectory: Directory does not exist: " + DirectoryPath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	// Paths returned by GetFilesInDirectory start with DirectoryPath, so the rest of the path is relative to it.
	const std::vector<std::string> Files = FILE_SYSTEM.GetFilesInDirectory(DirectoryPath, true);
	std::vector<FEUUID> ImportedAssets;
	for (size_t i = 0; i < Files.size(); i++)
	{
		FEAssetPackageEntryInitializeData EntryData;
		EntryData.Name = ToSafeRelativePath(Files[i].substr(DirectoryPath.size()));

		const FEUUID NewAssetID = EntryData.Name.empty() ? FEUUID() : ImportAssetFromFile(Files[i], EntryData);
		if (UNIQUE_ID.IsNull(NewAssetID))
		{
			LOG.Add("FEAssetPackage::ImportDirectory: Could not import file: " + Files[i], "FE_ASSET_PACKAGE", FE_LOG_ERROR);

			// Assets are removed in reverse order, so each removed asset is at the end of data.
			for (size_t j = ImportedAssets.size(); j > 0; j--)
				RemoveAsset(ImportedAssets[j - 1]);

			return false;
		}

		ImportedAssets.push_back(NewAssetID);
	}

	return true;
}

bool FEAssetPackage::ExportToDirectory(const std::string& DirectoryPath)
{
	std::vector<std::pair<FEUUID, std::string>> AssetsToExport;
	// File names on Windows are case insensitive, so assets with names that differ only in case would overwrite each other.
	std::unordered_map<std::string, std::string> LowerCaseNameToName;
	for (const auto& Entry : Header.Entries)
	{
		const std::string RelativePath = ToSafeRelativePath(Entry.second.Name);
		if (RelativePath.empty())
		{
			LOG.Add("FEAssetPackage::ExportToDirectory: Asset name \"" + Entry.second.Name + "\" is not a safe relative path, nothing was exported.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
			return false;
		}

		std::string LowerCaseName = RelativePath;
		std::transform(LowerCaseName.begin(), LowerCaseName.end(), LowerCaseName.begin(), [](unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
		const auto NameIterator = LowerCaseNameToName.find(LowerCaseName);
		if (NameIterator != LowerCaseNameToName.end())
		{
			LOG.Add("FEAssetPackage::ExportToDirectory: Assets \"" + NameIterator->second + "\" and \"" + RelativePath + "\" would be written to the same file, nothing was exported.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
			return false;
		}

		LowerCaseNameToName[LowerCaseName] = RelativePath;
		AssetsToExport.push_back({ Entry.first, RelativePath });
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(DirectoryPath) && !FILE_SYSTEM.MakeDirectory(DirectoryPath))
	{
		LOG.Add("FEAssetPackage::ExportToDirectory: Could not create directory: " + DirectoryPath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	for (size_t i = 0; i < AssetsToExport.size(); i++)
	{
		const std::string& RelativePath = AssetsToExport[i].second;
		const std::vector<std::string> FolderChain = FILE_SYSTEM.GetFolderChain(FILE_SYSTEM.GetDirectoryPath(RelativePath));
		for (size_t j = 0; j < FolderChain.size(); j++)
		{
			const std::string FolderPath = DirectoryPath + "/" + FolderChain[j];
			if (!FILE_SYSTEM.DoesDirectoryExist(FolderPath) && !FILE_SYSTEM.MakeDirectory(FolderPath))
			{
				LOG.Add("FEAssetPackage::ExportToDirectory: Could not create directory: " + FolderPath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
				return false;
			}
		}

		if (!ExportAssetToFile(AssetsToExport[i].first, DirectoryPath + "/" + RelativePath))
		{
			LOG.Add("FEAssetPackage::ExportToDirectory: Could not export asset \"" + RelativePath + "\" to " + DirectoryPath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

bool FEAssetPackage::ExportAssetToMemory(const FEUUID& ID, unsigned char*& RawData, size_t& Size)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::ExportAssetToMemory: Asset ID not present: " + UNIQUE_ID.ToString(ID), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackageAssetInfo& Entry = Header.Entries[ID];
	RawData = new unsigned char[Entry.Size];
	memcpy(RawData, Data.data() + Entry.Offset, Entry.Size);
	Size = Entry.Size;

	return true;
}

size_t FEAssetPackage::GetBuildTimeStamp()
{
	return Header.BuildTimeStamp;
}

std::string FEAssetPackage::GetBuildTimeStampAsString()
{
	return TIME.NanosecondTimeStampToDate(Header.BuildTimeStamp);
}

unsigned char* FEAssetPackage::ExportAsRawData(size_t& Size)
{
	Size = Header.Size + Data.size();
	if (Size == 0)
	{
		LOG.Add("FEAssetPackage::ExportAsRawData: Asset package size is 0.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return nullptr;
	}

	// First we will write the header.
	unsigned char* DataToReturn = new unsigned char[Size];
	unsigned char* CurrentDataPointer = DataToReturn;
	// Write the header start phrase.
	memcpy(CurrentDataPointer, HeaderStartPhrase.c_str(), HeaderStartPhrase.size());
	CurrentDataPointer += HeaderStartPhrase.size();
	// Write the header size.
	memcpy(CurrentDataPointer, &Header.Size, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);
	// Write the format version.
	memcpy(CurrentDataPointer, &Header.FormatVersion, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);
	// Write the build time stamp.
	memcpy(CurrentDataPointer, &Header.BuildTimeStamp, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);
	// Write the number of entries.
	memcpy(CurrentDataPointer, &Header.EntriesCount, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);

	for (auto& Entry : Header.Entries)
	{
		// Write the ID.
		const std::string IDString = UNIQUE_ID.ToString(Entry.second.ID);
		size_t IDSize = IDString.size();
		memcpy(CurrentDataPointer, &IDSize, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		memcpy(CurrentDataPointer, IDString.c_str(), IDSize);
		CurrentDataPointer += IDSize;

		// Write the name.
		size_t NameSize = Entry.second.Name.size();
		memcpy(CurrentDataPointer, &NameSize, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		memcpy(CurrentDataPointer, Entry.second.Name.c_str(), NameSize);
		CurrentDataPointer += NameSize;

		// Write the type.
		size_t TypeSize = Entry.second.Type.size();
		memcpy(CurrentDataPointer, &TypeSize, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		memcpy(CurrentDataPointer, Entry.second.Type.c_str(), TypeSize);
		CurrentDataPointer += TypeSize;

		// Write the tag.
		size_t TagSize = Entry.second.Tag.size();
		memcpy(CurrentDataPointer, &TagSize, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		memcpy(CurrentDataPointer, Entry.second.Tag.c_str(), TagSize);
		CurrentDataPointer += TagSize;

		// Write the comment.
		size_t CommentSize = Entry.second.Comment.size();
		memcpy(CurrentDataPointer, &CommentSize, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		memcpy(CurrentDataPointer, Entry.second.Comment.c_str(), CommentSize);
		CurrentDataPointer += CommentSize;

		// Write the time stamp.
		memcpy(CurrentDataPointer, &Entry.second.TimeStamp, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		// Write the size.
		memcpy(CurrentDataPointer, &Entry.second.Size, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		// Write the offset.
		memcpy(CurrentDataPointer, &Entry.second.Offset, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
	}

	// Now we will write the asset package data.
	memcpy(CurrentDataPointer, Data.data(), Data.size());

	return DataToReturn;
}

bool FEAssetPackage::LoadFromMemory(unsigned char* RawData, size_t Size)
{
	Header = FEAssetPackageHeader();
	Data.clear();

	if (RawData == nullptr)
	{
		LOG.Add("FEAssetPackage::LoadFromMemory: RawData is nullptr.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	unsigned char* CurrentDataPointer = RawData;
	size_t BytesLeft = Size;

	// Package could be truncated or corrupted, so every read is checked against remaining size.
	auto ReadBytes = [&](void* Destination, size_t Count) -> bool
	{
		if (Count > BytesLeft)
			return false;

		memcpy(Destination, CurrentDataPointer, Count);
		CurrentDataPointer += Count;
		BytesLeft -= Count;
		return true;
	};

	auto ReadString = [&](std::string& Destination) -> bool
	{
		size_t StringSize = 0;
		if (!ReadBytes(&StringSize, sizeof(size_t)) || StringSize > BytesLeft)
			return false;

		Destination = std::string((char*)CurrentDataPointer, StringSize);
		CurrentDataPointer += StringSize;
		BytesLeft -= StringSize;
		return true;
	};

	auto Fail = [&](const std::string& Message) -> bool
	{
		LOG.Add("FEAssetPackage::LoadFromMemory: " + Message, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		Header = FEAssetPackageHeader();
		Data.clear();
		return false;
	};

	const size_t HeaderStartPhraseSize = HeaderStartPhrase.size();
	if (Size < HeaderStartPhraseSize || memcmp(CurrentDataPointer, HeaderStartPhrase.c_str(), HeaderStartPhraseSize) != 0)
		return Fail("Header start phrase is not correct.");

	CurrentDataPointer += HeaderStartPhraseSize;
	BytesLeft -= HeaderStartPhraseSize;

	if (!ReadBytes(&Header.Size, sizeof(size_t)) || !ReadBytes(&Header.FormatVersion, sizeof(size_t)) ||
		!ReadBytes(&Header.BuildTimeStamp, sizeof(size_t)) || !ReadBytes(&Header.EntriesCount, sizeof(size_t)))
		return Fail("Package is too small to contain header.");

	if (Header.Size > Size)
		return Fail("Header size is bigger than package size.");

	for (size_t i = 0; i < Header.EntriesCount; i++)
	{
		FEAssetPackageAssetInfo NewEntry;

		std::string IDString;
		if (!ReadString(IDString))
			return Fail("Asset entry is out of package bounds.");

		NewEntry.ID = UNIQUE_ID.FromString(IDString);
		if (UNIQUE_ID.IsNull(NewEntry.ID) || UNIQUE_ID.ToString(NewEntry.ID).size() != IDString.size())
			return Fail("Asset ID is not a valid UUID: " + IDString);

		if (Header.Entries.find(NewEntry.ID) != Header.Entries.end())
			return Fail("Asset ID is not unique: " + IDString);

		if (!ReadString(NewEntry.Name) || !ReadString(NewEntry.Type) || !ReadString(NewEntry.Tag) || !ReadString(NewEntry.Comment) ||
			!ReadBytes(&NewEntry.TimeStamp, sizeof(size_t)) || !ReadBytes(&NewEntry.Size, sizeof(size_t)) || !ReadBytes(&NewEntry.Offset, sizeof(size_t)))
			return Fail("Asset entry is out of package bounds.");

		Header.Entries[NewEntry.ID] = NewEntry;
	}

	if (Size - BytesLeft != Header.Size)
		return Fail("Header size does not match header content.");

	const size_t DataSize = Size - Header.Size;
	for (const auto& Entry : Header.Entries)
	{
		if (Entry.second.Offset > DataSize || Entry.second.Size > DataSize - Entry.second.Offset)
			return Fail("Asset " + UNIQUE_ID.ToString(Entry.first) + " is out of package data bounds.");
	}

	Data.resize(DataSize);
	memcpy(Data.data(), CurrentDataPointer, DataSize);
	return true;
}

std::vector<FEUUID> FEAssetPackage::GetAssetIDsByName(const std::string& Name)
{
	std::vector<FEUUID> Result;
	for (auto& Entry : Header.Entries)
	{
		if (Entry.second.Name == Name)
			Result.push_back(Entry.first);
	}

	return Result;
}