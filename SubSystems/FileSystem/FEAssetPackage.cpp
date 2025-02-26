#include "FEAssetPackage.h"
#include "../ResourceManager/FEResourceManager.h"
#include "../SubSystems/Scene/FESceneManager.h"
using namespace FocalEngine;

std::string FEAssetPackage::HeaderStartPhrase = "FEAssetPackage";
FEAssetPackage::FEAssetPackage() : FEObject(FE_OBJECT_TYPE::FE_ASSET_PACKAGE, "Unnamed asset package") {}

FEAssetPackage::FEAssetPackage(std::string PackageName, std::vector<std::string> FilesToAdd) : FEObject(FE_OBJECT_TYPE::FE_ASSET_PACKAGE, PackageName)
{
	for (size_t i = 0; i < FilesToAdd.size(); i++)
	{
		if (ImportAssetFromFile(FilesToAdd[i]).empty())
		{
			LOG.Add("FEAssetPackage::FEAssetPackage: Could not add file to the package: " + FilesToAdd[i], "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		}
	}
}

bool FEAssetPackage::IsAssetIDPresent(const std::string& ID)
{
	return Header.Entries.find(ID) != Header.Entries.end();
}

std::string FEAssetPackage::ImportAssetFromFile(const std::string& FilePath, FEAssetPackageEntryIntializeData IntializeData)
{
	// For time being, we will not support hierarchical asset packages.
	// Because of this, we will not support adding directories to the asset package.
	if (FILE_SYSTEM.DoesDirectoryExist(FilePath))
	{
		LOG.Add("FEAssetPackage::AddFile: Directories are not supported in asset packages: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
	}

	// First check if file exists.
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEAssetPackage::AddFile: File does not exist: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
	}

	std::string IDToUse = IntializeData.IsEmpty() ? APPLICATION.GetUniqueHexID() : IntializeData.ID.empty() ? APPLICATION.GetUniqueHexID() : IntializeData.ID;
	if (IsAssetIDPresent(IDToUse))
	{
		LOG.Add("FEAssetPackage::AddFile: Asset ID already present: " + IDToUse, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
	}

	std::string NameToUse = IntializeData.IsEmpty() ? FILE_SYSTEM.GetFileName(FilePath) : IntializeData.Name.empty() ? FILE_SYSTEM.GetFileName(FilePath) : IntializeData.Name;
	std::string TypeToUse = IntializeData.IsEmpty() ? "" : IntializeData.Type;
	std::string TagToUse = IntializeData.IsEmpty() ? "" : IntializeData.Tag;
	std::string CommentToUse = IntializeData.IsEmpty() ? "" : IntializeData.Comment;

	// Now we will try to read raw data from the file.
	std::ifstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("FEAssetPackage::AddFile: Could not open file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
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

std::string FEAssetPackage::ImportAssetFromMemory(unsigned char* RawData, size_t Size, FEAssetPackageEntryIntializeData IntializeData)
{
	std::string IDToUse = IntializeData.IsEmpty() ? APPLICATION.GetUniqueHexID() : IntializeData.ID.empty() ? APPLICATION.GetUniqueHexID() : IntializeData.ID;
	if (IsAssetIDPresent(IDToUse))
	{
		LOG.Add("FEAssetPackage::AddFile: Asset ID already present: " + IDToUse, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
	}

	std::string NameToUse = IntializeData.IsEmpty() ? "" : IntializeData.Name;
	std::string TypeToUse = IntializeData.IsEmpty() ? "" : IntializeData.Type;
	std::string TagToUse = IntializeData.IsEmpty() ? "" : IntializeData.Tag;
	std::string CommentToUse = IntializeData.IsEmpty() ? "" : IntializeData.Comment;

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

std::string FEAssetPackage::ImportAsset(FEObject* Object, FEAssetPackageEntryIntializeData IntializeData)
{
	if (Object == nullptr)
	{
		LOG.Add("FEAssetPackage::ImportAsset: Object is nullptr.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
	}

	std::string IDToUse = IntializeData.IsEmpty() ? APPLICATION.GetUniqueHexID() : IntializeData.ID.empty() ? APPLICATION.GetUniqueHexID() : IntializeData.ID;
	if (IsAssetIDPresent(IDToUse))
	{
		LOG.Add("FEAssetPackage::ImportAsset: Asset ID already present: " + IDToUse, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
	}

	std::string NameToUse = IntializeData.IsEmpty() ? Object->GetName() : IntializeData.Name.empty() ? Object->GetName() : IntializeData.Name;
	std::string TypeToUse = IntializeData.IsEmpty() ? "" : IntializeData.Type;
	std::string TagToUse = IntializeData.IsEmpty() ? Object->GetTag() : IntializeData.Tag;
	std::string CommentToUse = IntializeData.IsEmpty() ? "" : IntializeData.Comment;

	// TODO: This should save info to memory, not to file.
	switch (Object->GetType())
	{
		case FE_OBJECT_TYPE::FE_TEXTURE:
		{
			FETexture* Texture = reinterpret_cast<FETexture*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_TEXTURE";

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempTexture.texture";
			RESOURCE_MANAGER.SaveFETexture(Texture, FilePath.c_str());
			std::string ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.DeleteFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_MESH:
		{
			FEMesh* Mesh = reinterpret_cast<FEMesh*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_MESH";

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempMesh.mesh";
			RESOURCE_MANAGER.SaveFEMesh(Mesh, FilePath.c_str());
			std::string ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.DeleteFile(FilePath);

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

			std::string ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.DeleteFile(FilePath);

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

			std::string ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.DeleteFile(FilePath);

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

			std::string ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.DeleteFile(FilePath);

			return ResultingID;
		}

		case FE_OBJECT_TYPE::FE_NATIVE_SCRIPT_MODULE:
		{
			FENativeScriptModule* NativeScriptModule = reinterpret_cast<FENativeScriptModule*>(Object);

			if (TypeToUse.empty())
				TypeToUse = "FE_NATIVE_SCRIPT_MODULE";

			std::string FilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "TempNativeScriptModule.nativescriptmodule";
			RESOURCE_MANAGER.SaveFENativeScriptModule(NativeScriptModule, FilePath);
			std::string ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.DeleteFile(FilePath);

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

			std::string ResultingID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ IDToUse, NameToUse, TypeToUse, TagToUse, CommentToUse });
			FILE_SYSTEM.DeleteFile(FilePath);

			return ResultingID;
		}

		default:
		{
			LOG.Add("FEAssetPackage::ImportAsset: Object type not supported: " + Object->GetName(), "FE_ASSET_PACKAGE", FE_LOG_ERROR);
			return "";
		}
	}

	return "";
}

bool FEAssetPackage::RemoveAsset(const std::string& ID)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::RemoveFile: Asset ID not present: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
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

bool FEAssetPackage::UpdateAssetFromFile(const std::string& ID, const std::string& FilePath)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::UpdateFile: Asset ID not present: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEAssetPackage::UpdateFile: File does not exist: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackageAssetInfo OldEntryInfo = Header.Entries[ID];
	if (!RemoveAsset(ID))
	{
		LOG.Add("FEAssetPackage::UpdateFile: Could not remove old asset: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	std::string NewID = ImportAssetFromFile(FilePath, FEAssetPackageEntryIntializeData{ OldEntryInfo.ID, OldEntryInfo.Name, OldEntryInfo.Type, OldEntryInfo.Tag, OldEntryInfo.Comment });
	if (NewID.empty())
	{
		LOG.Add("FEAssetPackage::UpdateFile: Could not import file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	if (NewID != ID)
	{
		LOG.Add("FEAssetPackage::UpdateFile: New ID is different from the old ID.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEAssetPackage::UpdateAssetFromMemory(const std::string& ID, unsigned char* RawData, size_t Size)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::UpdateFile: Asset ID not present: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackageAssetInfo OldEntryInfo = Header.Entries[ID];
	if (!RemoveAsset(ID))
	{
		LOG.Add("FEAssetPackage::UpdateFile: Could not remove old asset: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	std::string NewID = ImportAssetFromMemory(RawData, Size, FEAssetPackageEntryIntializeData{ OldEntryInfo.ID, OldEntryInfo.Name, OldEntryInfo.Type, OldEntryInfo.Tag, OldEntryInfo.Comment });
	if (NewID.empty())
	{
		LOG.Add("FEAssetPackage::UpdateFile: Could not import file from memory.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	if (NewID != ID)
	{
		LOG.Add("FEAssetPackage::UpdateFile: New ID is different from the old ID.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

void FEAssetPackage::UpdateHeaderSize()
{
	// Update header time stamp.
	Header.BuildTimeStamp = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	Header.Size = HeaderStartPhrase.size();
	// Size of variable that holds the header size.
	Header.Size += sizeof(size_t);
	// Size of variable that holds the format version.
	Header.Size += sizeof(size_t);
	// Size of variable that holds the build time stamp.
	Header.Size += sizeof(size_t);
	// Size of variable that holds the number of assets.
	Header.Size += sizeof(size_t);

	// Now will go through all the asset package entries and add their sizes.
	for (auto& Entry : Header.Entries)
	{
		Header.Size += sizeof(size_t);
		Header.Size += Entry.second.ID.size();
		Header.Size += sizeof(size_t);
		Header.Size += Entry.second.Name.size();
		Header.Size += sizeof(size_t);
		Header.Size += Entry.second.Type.size();
		Header.Size += sizeof(size_t);
		Header.Size += Entry.second.Tag.size();
		Header.Size += sizeof(size_t);
		Header.Size += Entry.second.Comment.size();

		// Size of variable that holds the size, offset of asset and time stamp.
		Header.Size += sizeof(size_t) * 3;
	}
}

FEAssetPackage::~FEAssetPackage() {}

bool FEAssetPackage::SaveToFile(const std::string& FilePath)
{
	std::ofstream File(FilePath, std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add("FEAssetPackage::WriteToFile: Could not open file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return false;
	}

	// Here we will use ExportAsRawData to get the raw data of the asset package.
	size_t DataSize = 0;
	unsigned char* RawData = ExportAsRawData(DataSize);
	if (RawData == nullptr)
	{
		LOG.Add("FEAssetPackage::WriteToFile: Could not export asset package as raw data.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		File.close();
		FILE_SYSTEM.DeleteFile(FilePath);
		return false;
	}

	if (DataSize == 0)
	{
		LOG.Add("FEAssetPackage::WriteToFile: Asset package size is 0.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		File.close();
		delete[] RawData;
		FILE_SYSTEM.DeleteFile(FilePath);
		return false;
	}

	File.write((char*)RawData, DataSize);
	delete[] RawData;

	// Sanity check, if file written data is not equal to the header size, log an error.
	size_t WrittenDataSize = File.tellp();
	if (WrittenDataSize != Header.Size + Data.size())
	{
		LOG.Add("FEAssetPackage::WriteToFile: package size is not equal to the written data size.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		File.close();
		FILE_SYSTEM.DeleteFile(FilePath);
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

	if (!LoadFromMemory(RawData, FileSize))
	{
		LOG.Add("FEAssetPackage::LoadFromFile: Could not properly load asset package from file: " + FilePath, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		delete[] RawData;
		return false;
	}

	return true;
}

FEAssetPackageAssetInfo FEAssetPackage::GetAssetInfo(const std::string& ID)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::GetAssetInfo: Asset ID not present: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
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

char* FEAssetPackage::GetAssetDataCopy(const std::string& ID)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::GetAssetDataCopy: Asset ID not present: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return nullptr;
	}

	FEAssetPackageAssetInfo& Entry = Header.Entries[ID];
	char* DataToReturn = new char[Entry.Size];
	memcpy(DataToReturn, Data.data() + Entry.Offset, Entry.Size);

	return DataToReturn;
}

bool FEAssetPackage::ExportAssetToFile(const std::string& ID, const std::string& FilePath)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::ExportAssetToFile: Asset ID not present: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
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
	return true;
}

bool FEAssetPackage::ExportAssetToMemory(const std::string& ID, unsigned char*& RawData, size_t& Size)
{
	if (!IsAssetIDPresent(ID))
	{
		LOG.Add("FEAssetPackage::ExportAssetToMemory: Asset ID not present: " + ID, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
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
		size_t IDSize = Entry.second.ID.size();
		memcpy(CurrentDataPointer, &IDSize, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		memcpy(CurrentDataPointer, Entry.second.ID.c_str(), IDSize);
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
	// First we will read the header.
	unsigned char* CurrentDataPointer = RawData;
	size_t HeaderStartPhraseSize = HeaderStartPhrase.size();
	if (memcmp(CurrentDataPointer, HeaderStartPhrase.c_str(), HeaderStartPhraseSize) != 0)
	{
		LOG.Add("FEAssetPackage::LoadFromMemory: Header start phrase is not correct.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return  false;
	}
	CurrentDataPointer += HeaderStartPhraseSize;

	// Read the header size.
	memcpy(&Header.Size, CurrentDataPointer, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);
	// Read the format version.
	memcpy(&Header.FormatVersion, CurrentDataPointer, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);
	// Read the build time stamp.
	memcpy(&Header.BuildTimeStamp, CurrentDataPointer, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);
	// Read the number of entries.
	memcpy(&Header.EntriesCount, CurrentDataPointer, sizeof(size_t));
	CurrentDataPointer += sizeof(size_t);

	for (size_t i = 0; i < Header.EntriesCount; i++)
	{
		FEAssetPackageAssetInfo NewEntry;

		// Read the ID.
		size_t IDSize = 0;
		memcpy(&IDSize, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		NewEntry.ID = std::string((char*)CurrentDataPointer, IDSize);
		CurrentDataPointer += IDSize;

		// Read the name.
		size_t NameSize = 0;
		memcpy(&NameSize, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		NewEntry.Name = std::string((char*)CurrentDataPointer, NameSize);
		CurrentDataPointer += NameSize;

		// Read the type.
		size_t TypeSize = 0;
		memcpy(&TypeSize, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		NewEntry.Type = std::string((char*)CurrentDataPointer, TypeSize);
		CurrentDataPointer += TypeSize;

		// Read the tag.
		size_t TagSize = 0;
		memcpy(&TagSize, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		NewEntry.Tag = std::string((char*)CurrentDataPointer, TagSize);
		CurrentDataPointer += TagSize;

		// Read the comment.
		size_t CommentSize = 0;
		memcpy(&CommentSize, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		NewEntry.Comment = std::string((char*)CurrentDataPointer, CommentSize);
		CurrentDataPointer += CommentSize;

		// Read the time stamp.
		memcpy(&NewEntry.TimeStamp, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		// Read the size.
		memcpy(&NewEntry.Size, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);
		// Read the offset.
		memcpy(&NewEntry.Offset, CurrentDataPointer, sizeof(size_t));
		CurrentDataPointer += sizeof(size_t);

		Header.Entries[NewEntry.ID] = NewEntry;
	}

	// Now we will read the asset package data.
	Data.resize(Size - Header.Size);
	memcpy(Data.data(), CurrentDataPointer, Size - Header.Size);

	return true;
}

std::vector<std::string> FEAssetPackage::GetAssetIDsByName(const std::string& Name)
{
	std::vector<std::string> Result;
	for (auto& Entry : Header.Entries)
	{
		if (Entry.second.Name == Name)
			Result.push_back(Entry.first);
	}

	return Result;
}