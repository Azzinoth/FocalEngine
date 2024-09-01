#include "FEAssetPackage.h"
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
	std::string NameToUse = IntializeData.IsEmpty() ? FILE_SYSTEM.GetFileName(FilePath) : IntializeData.Name.empty() ? FILE_SYSTEM.GetFileName(FilePath) : IntializeData.Name;

	if (IsAssetIDPresent(IDToUse))
	{
		LOG.Add("FEAssetPackage::AddFile: Asset ID already present: " + IDToUse, "FE_ASSET_PACKAGE", FE_LOG_ERROR);
		return "";
	}

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
	NewEntry.Offset = Header.CurrentAssetOffset;
	NewEntry.Type = "";
	NewEntry.Tag = "";
	NewEntry.Comment = "";

	Header.Entries[NewEntry.ID] = NewEntry;

	Header.EntriesCount++;
	Header.CurrentAssetOffset += NewEntry.Size;
	UpdateHeaderSize();

	// And finally, add the file data to the asset package data.
	Data.insert(Data.end(), FileData.begin(), FileData.end());

	return IDToUse;
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

//void FEAssetPackage::WriteHeader(std::ofstream& OpenedFile)
//{
//	if (!OpenedFile.is_open())
//	{
//		LOG.Add("FEAssetPackage::WriteHeader: File is not open.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
//		return;
//	}
//
//	// Write the header start phrase.
//	OpenedFile.write(HeaderStartPhrase.c_str(), HeaderStartPhrase.size());
//
//	// Write the header size.
//	OpenedFile.write((char*)&Header.Size, sizeof(size_t));
//
//	// Write the format version.
//	OpenedFile.write((char*)&Header.FormatVersion, sizeof(size_t));
//
//	// Write the build time stamp.
//	OpenedFile.write((char*)&Header.BuildTimeStamp, sizeof(size_t));
//
//	// Write the number of entries.
//	OpenedFile.write((char*)&Header.EntriesCount, sizeof(size_t));
//
//	// Write the asset package entries.
//	for (auto& Entry : Header.Entries)
//	{
//		// Write the ID.
//		size_t IDSize = Entry.second.ID.size();
//		OpenedFile.write((char*)&IDSize, sizeof(size_t));
//		OpenedFile.write(Entry.second.ID.c_str(), IDSize);
//
//		// Write the name.
//		size_t NameSize = Entry.second.Name.size();
//		OpenedFile.write((char*)&NameSize, sizeof(size_t));
//		OpenedFile.write(Entry.second.Name.c_str(), NameSize);
//
//		// Write the type.
//		size_t TypeSize = Entry.second.Type.size();
//		OpenedFile.write((char*)&TypeSize, sizeof(size_t));
//		OpenedFile.write(Entry.second.Type.c_str(), TypeSize);
//
//		// Write the tag.
//		size_t TagSize = Entry.second.Tag.size();
//		OpenedFile.write((char*)&TagSize, sizeof(size_t));
//		OpenedFile.write(Entry.second.Tag.c_str(), TagSize);
//
//		// Write the comment.
//		size_t CommentSize = Entry.second.Comment.size();
//		OpenedFile.write((char*)&CommentSize, sizeof(size_t));
//		OpenedFile.write(Entry.second.Comment.c_str(), CommentSize);
//
//		// Write the time stamp.
//		OpenedFile.write((char*)&Entry.second.TimeStamp, sizeof(size_t));
//
//		// Write the size.
//		OpenedFile.write((char*)&Entry.second.Size, sizeof(size_t));
//
//		// Write the offset.
//		OpenedFile.write((char*)&Entry.second.Offset, sizeof(size_t));
//	}
//}

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

	

	//WriteHeader(File);
	//// Sanity check, if file written data is not equal to the header size, log an error.
	//size_t WrittenDataSize = File.tellp();
	//if (WrittenDataSize != Header.Size)
	//{
	//	LOG.Add("FEAssetPackage::WriteToFile: Header size is not equal to the written data size.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
	//	File.close();
	//	FILE_SYSTEM.DeleteFile(FilePath);
	//	return false;
	//}

	//// Write the asset package data.
	//File.write(Data.data(), Data.size());
	return true;
}

//bool FEAssetPackage::ReadHeader(std::ifstream& OpenedFile)
//{
//	if (!OpenedFile.is_open())
//	{
//		LOG.Add("FEAssetPackage::ReadHeader: File is not open.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
//		return false;
//	}
//
//	// Read the header start phrase.
//	char* HeaderStartPhraseBuffer = new char [HeaderStartPhrase.size() + 1];
//	OpenedFile.read(HeaderStartPhraseBuffer, HeaderStartPhrase.size());
//	HeaderStartPhraseBuffer[HeaderStartPhrase.size()] = '\0';
//	std::string HeaderStartPhraseRead(HeaderStartPhraseBuffer, HeaderStartPhrase.size());
//	if (HeaderStartPhraseRead != HeaderStartPhrase)
//	{
//		LOG.Add("FEAssetPackage::ReadHeader: Header start phrase is not correct.", "FE_ASSET_PACKAGE", FE_LOG_ERROR);
//		delete[] HeaderStartPhraseBuffer;
//		return false;
//	}
//	delete[] HeaderStartPhraseBuffer;
//
//	// Read the header size.
//	OpenedFile.read((char*)&Header.Size, sizeof(size_t));
//
//	// Read the format version.
//	OpenedFile.read((char*)&Header.FormatVersion, sizeof(size_t));
//
//	// Read the build time stamp.
//	OpenedFile.read((char*)&Header.BuildTimeStamp, sizeof(size_t));
//
//	// Read the number of entries.
//	OpenedFile.read((char*)&Header.EntriesCount, sizeof(size_t));
//
//	// Read the asset package entries.
//	for (size_t i = 0; i < Header.EntriesCount; i++)
//	{
//		FEAssetPackageEntry NewEntry;
//
//		// Read the ID.
//		size_t IDSize = 0;
//		OpenedFile.read((char*)&IDSize, sizeof(size_t));
//		char* IDBuffer = new char[IDSize];
//		OpenedFile.read(IDBuffer, IDSize);
//		NewEntry.ID = std::string(IDBuffer, IDSize);
//		delete[] IDBuffer;
//
//		// Read the name.
//		size_t NameSize = 0;
//		OpenedFile.read((char*)&NameSize, sizeof(size_t));
//		char* NameBuffer = new char[NameSize];
//		OpenedFile.read(NameBuffer, NameSize);
//		NewEntry.Name = std::string(NameBuffer, NameSize);
//		delete[] NameBuffer;
//
//		// Read the type.
//		size_t TypeSize = 0;
//		OpenedFile.read((char*)&TypeSize, sizeof(size_t));
//		char* TypeBuffer = new char[TypeSize];
//		OpenedFile.read(TypeBuffer, TypeSize);
//		NewEntry.Type = std::string(TypeBuffer, TypeSize);
//		delete[] TypeBuffer;
//
//		// Read the tag.
//		size_t TagSize = 0;
//		OpenedFile.read((char*)&TagSize, sizeof(size_t));
//		char* TagBuffer = new char[TagSize];
//		OpenedFile.read(TagBuffer, TagSize);
//		NewEntry.Tag = std::string(TagBuffer, TagSize);
//		delete[] TagBuffer;
//
//		// Read the comment.
//		size_t CommentSize = 0;
//		OpenedFile.read((char*)&CommentSize, sizeof(size_t));
//		char* CommentBuffer = new char[CommentSize];
//		OpenedFile.read(CommentBuffer, CommentSize);
//		NewEntry.Comment = std::string(CommentBuffer, CommentSize);
//
//		// Read the time stamp.
//		OpenedFile.read((char*)&NewEntry.TimeStamp, sizeof(size_t));
//
//		// Read the size.
//		OpenedFile.read((char*)&NewEntry.Size, sizeof(size_t));
//
//		// Read the offset.
//		OpenedFile.read((char*)&NewEntry.Offset, sizeof(size_t));
//
//		Header.Entries[NewEntry.ID] = NewEntry;
//	}
//
//	return true;
//}

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

	//// Try to read the header.
	//if (!ReadHeader(File))
	//{
	//	File.close();
	//	return false;
	//}

	//// Read the asset package data.
	//File.seekg(Header.Size, std::ios::beg);
	//size_t AssetPackageDataSize = FILE_SYSTEM.GetFileSize(FilePath) - Header.Size;
	//Data.resize(AssetPackageDataSize);
	//File.read(Data.data(), AssetPackageDataSize);

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