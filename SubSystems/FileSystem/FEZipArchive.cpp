#include "FEZipArchive.h"
#include "../../ThirdParty/lodepng/lodepng.h"
using namespace FocalEngine;

static const uint32_t LocalFileHeaderSignature = 0x04034b50;
static const uint32_t CentralDirectoryHeaderSignature = 0x02014b50;
static const uint32_t EndOfCentralDirectorySignature = 0x06054b50;
static const size_t LocalFileHeaderSize = 30;
static const size_t CentralDirectoryHeaderSize = 46;
static const size_t EndOfCentralDirectorySize = 22;
static const size_t MaximumCommentSize = 0xFFFF;

static uint16_t ReadUInt16(const std::vector<unsigned char>& Data, const size_t Offset)
{
	return static_cast<uint16_t>(Data[Offset] | (Data[Offset + 1] << 8));
}

static uint32_t ReadUInt32(const std::vector<unsigned char>& Data, const size_t Offset)
{
	return static_cast<uint32_t>(Data[Offset]) | (static_cast<uint32_t>(Data[Offset + 1]) << 8) | (static_cast<uint32_t>(Data[Offset + 2]) << 16) | (static_cast<uint32_t>(Data[Offset + 3]) << 24);
}

bool FEZipArchive::ReadFiles(const std::string& FilePath, std::vector<FEZipArchiveFile>& OutFiles, std::string& OutErrorMessage, const std::function<bool(const std::string& FileName)>& ShouldReadFile)
{
	OutFiles.clear();
	auto Fail = [&](const std::string& Reason) -> bool
	{
		OutFiles.clear();
		OutErrorMessage = "\"" + FilePath + "\" " + Reason;
		return false;
	};

	std::ifstream File(FilePath, std::ios::binary | std::ios::ate);
	if (!File.is_open())
		return Fail("could not be opened");

	const std::streamoff FileSize = File.tellg();
	if (FileSize < static_cast<std::streamoff>(EndOfCentralDirectorySize))
		return Fail("is not a zip archive");

	std::vector<unsigned char> Archive(static_cast<size_t>(FileSize));
	File.seekg(0, std::ios::beg);
	if (!File.read(reinterpret_cast<char*>(Archive.data()), FileSize))
		return Fail("could not be read");
	File.close();

	// End of central directory record is followed only by archive comment, whose length is stored in the record.
	size_t EndRecordOffset = std::string::npos;
	const size_t LastPossibleOffset = Archive.size() - EndOfCentralDirectorySize;
	const size_t FirstPossibleOffset = LastPossibleOffset > MaximumCommentSize ? LastPossibleOffset - MaximumCommentSize : 0;
	for (size_t Offset = LastPossibleOffset + 1; Offset-- > FirstPossibleOffset;)
	{
		if (ReadUInt32(Archive, Offset) == EndOfCentralDirectorySignature && Offset + EndOfCentralDirectorySize + ReadUInt16(Archive, Offset + 20) == Archive.size())
		{
			EndRecordOffset = Offset;
			break;
		}
	}

	if (EndRecordOffset == std::string::npos)
		return Fail("is not a zip archive");

	const uint16_t DiskNumber = ReadUInt16(Archive, EndRecordOffset + 4);
	const uint16_t CentralDirectoryDisk = ReadUInt16(Archive, EndRecordOffset + 6);
	const uint16_t EntryCountOnDisk = ReadUInt16(Archive, EndRecordOffset + 8);
	const uint16_t EntryCount = ReadUInt16(Archive, EndRecordOffset + 10);
	const uint32_t CentralDirectorySize = ReadUInt32(Archive, EndRecordOffset + 12);
	const uint32_t CentralDirectoryOffset = ReadUInt32(Archive, EndRecordOffset + 16);

	if (EntryCount == 0xFFFF || CentralDirectorySize == 0xFFFFFFFF || CentralDirectoryOffset == 0xFFFFFFFF)
		return Fail("uses ZIP64, which is not supported");

	if (DiskNumber != 0 || CentralDirectoryDisk != 0 || EntryCountOnDisk != EntryCount)
		return Fail("is split into several parts, which is not supported");

	const size_t CentralDirectoryEnd = static_cast<size_t>(CentralDirectoryOffset) + CentralDirectorySize;
	if (CentralDirectoryEnd > EndRecordOffset)
		return Fail("is damaged, its central directory is outside of archive");

	size_t Position = CentralDirectoryOffset;
	for (uint16_t i = 0; i < EntryCount; i++)
	{
		if (Position + CentralDirectoryHeaderSize > CentralDirectoryEnd || ReadUInt32(Archive, Position) != CentralDirectoryHeaderSignature)
			return Fail("is damaged, its central directory is invalid");

		const uint16_t Flags = ReadUInt16(Archive, Position + 8);
		const uint16_t CompressionMethod = ReadUInt16(Archive, Position + 10);
		const uint32_t ExpectedCRC = ReadUInt32(Archive, Position + 16);
		const uint32_t CompressedSize = ReadUInt32(Archive, Position + 20);
		const uint32_t UncompressedSize = ReadUInt32(Archive, Position + 24);
		const uint16_t NameLength = ReadUInt16(Archive, Position + 28);
		const uint16_t ExtraFieldLength = ReadUInt16(Archive, Position + 30);
		const uint16_t CommentLength = ReadUInt16(Archive, Position + 32);
		const uint32_t LocalHeaderOffset = ReadUInt32(Archive, Position + 42);

		const size_t NameOffset = Position + CentralDirectoryHeaderSize;
		Position = NameOffset + NameLength + ExtraFieldLength + CommentLength;
		if (Position > CentralDirectoryEnd)
			return Fail("is damaged, its central directory is invalid");

		std::string Name(reinterpret_cast<const char*>(Archive.data() + NameOffset), NameLength);
		std::replace(Name.begin(), Name.end(), '\\', '/');
		if (Name.empty() || Name.back() == '/')
			continue;

		if (ShouldReadFile && !ShouldReadFile(Name))
			continue;

		if (Flags & 0x1)
			return Fail("contains encrypted file \"" + Name + "\", encryption is not supported");

		if (CompressedSize == 0xFFFFFFFF || UncompressedSize == 0xFFFFFFFF || LocalHeaderOffset == 0xFFFFFFFF)
			return Fail("uses ZIP64, which is not supported");

		if (CompressionMethod != 0 && CompressionMethod != 8)
			return Fail("contains file \"" + Name + "\" compressed with method " + std::to_string(CompressionMethod) + ", only stored and deflate compressed files are supported");

		if (static_cast<size_t>(LocalHeaderOffset) + LocalFileHeaderSize > CentralDirectoryOffset || ReadUInt32(Archive, LocalHeaderOffset) != LocalFileHeaderSignature)
			return Fail("is damaged, local header of \"" + Name + "\" is invalid");

		const size_t DataOffset = LocalHeaderOffset + LocalFileHeaderSize + ReadUInt16(Archive, LocalHeaderOffset + 26) + ReadUInt16(Archive, LocalHeaderOffset + 28);
		if (DataOffset + CompressedSize > CentralDirectoryOffset)
			return Fail("is damaged, data of \"" + Name + "\" is outside of archive");

		FEZipArchiveFile NewFile;
		NewFile.Name = Name;
		if (CompressionMethod == 0)
		{
			if (CompressedSize != UncompressedSize)
				return Fail("is damaged, sizes of \"" + Name + "\" do not match");

			NewFile.Data.assign(Archive.begin() + DataOffset, Archive.begin() + DataOffset + CompressedSize);
		}
		else
		{
			unsigned char* InflatedData = nullptr;
			size_t InflatedSize = 0;
			const unsigned Error = lodepng_inflate(&InflatedData, &InflatedSize, Archive.data() + DataOffset, CompressedSize, &lodepng_default_decompress_settings);
			if (Error == 0 && InflatedSize == UncompressedSize)
				NewFile.Data.assign(InflatedData, InflatedData + InflatedSize);
			free(InflatedData);

			if (Error != 0 || InflatedSize != UncompressedSize)
				return Fail("is damaged, \"" + Name + "\" could not be decompressed");
		}

		if (lodepng_crc32(NewFile.Data.data(), NewFile.Data.size()) != ExpectedCRC)
			return Fail("is damaged, checksum of \"" + Name + "\" does not match");

		OutFiles.push_back(std::move(NewFile));
	}

	return true;
}