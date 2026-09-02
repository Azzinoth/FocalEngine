#include "FELASLoader.h"
#include <cstring>
#include <cstdint>
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetLASLoader()
{
	return FELASLoader::GetInstancePointer();
}
#endif

void FELASData::FreeHeaderCopy()
{
	if (Header == nullptr)
		return;

	delete[] Header->user_data_in_header;

	if (Header->vlrs != nullptr)
	{
		for (laszip_U32 i = 0; i < Header->number_of_variable_length_records; i++)
			delete[] Header->vlrs[i].data;

		delete[] Header->vlrs;
	}

	delete[] Header->user_data_after_header;

	delete Header;
	Header = nullptr;
}

FELASData::~FELASData()
{
	FreeHeaderCopy();

	for (size_t i = 0; i < RawPoints.size(); i++)
		delete[] RawPoints[i].extra_bytes;
}

FELASLoader::FELASLoader()
{
	if (laszip_load_dll() != 0)
	{
		LOG.Add("DLL ERROR: loading LASzip DLL failed.", "FE_LOG_LOADING", FE_LOG_ERROR);
		bDLLPresent = false;
		LOG.Add("Loading LAS/LAZ files is disabled.", "FE_LOG_LOADING", FE_LOG_WARNING);
	}
	else
	{
		bDLLPresent = true;
	}
}

FELASLoader::~FELASLoader()
{
	ClearLoadedData();
}

bool FELASLoader::IsDLLPresent()
{
	return bDLLPresent;
}

laszip_header* FELASLoader::DeepCopyHeader(const laszip_header* Source)
{
	laszip_header* Copy = new laszip_header(*Source);

	Copy->user_data_in_header = nullptr;
	if (Source->user_data_in_header_size != 0 && Source->user_data_in_header != nullptr)
	{
		Copy->user_data_in_header = new laszip_U8[Source->user_data_in_header_size];
		std::memcpy(Copy->user_data_in_header, Source->user_data_in_header, Source->user_data_in_header_size);
	}

	Copy->vlrs = nullptr;
	if (Source->number_of_variable_length_records != 0 && Source->vlrs != nullptr)
	{
		Copy->vlrs = new laszip_vlr_struct[Source->number_of_variable_length_records];
		for (laszip_U32 i = 0; i < Source->number_of_variable_length_records; i++)
		{
			Copy->vlrs[i] = Source->vlrs[i];
			Copy->vlrs[i].data = nullptr;
			if (Source->vlrs[i].record_length_after_header != 0 && Source->vlrs[i].data != nullptr)
			{
				Copy->vlrs[i].data = new laszip_U8[Source->vlrs[i].record_length_after_header];
				std::memcpy(Copy->vlrs[i].data, Source->vlrs[i].data, Source->vlrs[i].record_length_after_header);
			}
		}
	}

	Copy->user_data_after_header = nullptr;
	if (Source->user_data_after_header_size != 0 && Source->user_data_after_header != nullptr)
	{
		Copy->user_data_after_header = new laszip_U8[Source->user_data_after_header_size];
		std::memcpy(Copy->user_data_after_header, Source->user_data_after_header, Source->user_data_after_header_size);
	}

	return Copy;
}

void FELASLoader::ClearLoadedData()
{
	if (LastLoadedData != nullptr)
	{
		delete LastLoadedData;
		LastLoadedData = nullptr;
	}
}

static void CloseAndDestroyReader(laszip_POINTER LaszipReader, bool bCloseReader)
{
	if (bCloseReader && laszip_close_reader(LaszipReader))
		LOG.Add("FELASLoader::ReadFile: closing laszip reader failed", "FE_LOG_LOADING", FE_LOG_ERROR);

	if (laszip_destroy(LaszipReader))
		LOG.Add("FELASLoader::ReadFile: destroying laszip reader failed", "FE_LOG_LOADING", FE_LOG_ERROR);
}

bool FELASLoader::ReadFile(const std::string& FilePath, bool bSaveHeaderCopy, bool bSaveRawPoints)
{
	ClearLoadedData();

	if (FilePath.empty())
	{
		LOG.Add("FELASLoader::ReadFile: FilePath is empty", "FE_LOG_LOADING", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FELASLoader::ReadFile: File does not exist", "FE_LOG_LOADING", FE_LOG_WARNING);
		return false;
	}

	if (!IsDLLPresent())
	{
		LOG.Add("FELASLoader::ReadFile: LAS/LAZ files are not enabled", "FE_LOG_LOADING", FE_LOG_WARNING);
		return false;
	}

	laszip_POINTER LaszipReader;
	laszip_I32 Error = laszip_create(&LaszipReader);
	if (Error)
	{
		LOG.Add("FELASLoader::ReadFile: Creating laszip reader failed with error: " + std::to_string(Error), "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	laszip_BOOL bIsCompressed = 0;
	bool bIsLASOrLAZFile = !laszip_open_reader(LaszipReader, FilePath.c_str(), &bIsCompressed);
	if (!bIsLASOrLAZFile)
	{
		LOG.Add("FELASLoader::ReadFile: File is not a valid LAS/LAZ file", "FE_LOG_LOADING", FE_LOG_ERROR);
		CloseAndDestroyReader(LaszipReader, false);
		return false;
	}

	FELASData* NewData = new FELASData();
	NewData->FilePath = FilePath;
	NewData->bWasCompressed = bIsCompressed;

	laszip_header* FileHeader;
	if (laszip_get_header_pointer(LaszipReader, &FileHeader))
	{
		LOG.Add("FELASLoader::ReadFile: getting header pointer from laszip reader failed", "FE_LOG_LOADING", FE_LOG_ERROR);
		CloseAndDestroyReader(LaszipReader, true);
		delete NewData;
		return false;
	}

	laszip_point* CurrentPointPointer;
	if (laszip_get_point_pointer(LaszipReader, &CurrentPointPointer))
	{
		LOG.Add("FELASLoader::ReadFile: getting point pointer from laszip reader failed", "FE_LOG_LOADING", FE_LOG_ERROR);
		CloseAndDestroyReader(LaszipReader, true);
		delete NewData;
		return false;
	}

	laszip_U64 PointCount = (FileHeader->number_of_point_records ? FileHeader->number_of_point_records : FileHeader->extended_number_of_point_records);
	if (PointCount == 0)
	{
		LOG.Add("FELASLoader::ReadFile: Point count is zero", "FE_LOG_LOADING", FE_LOG_ERROR);
		CloseAndDestroyReader(LaszipReader, true);
		delete NewData;
		return false;
	}

	if (bSaveHeaderCopy)
		NewData->Header = DeepCopyHeader(FileHeader);

	NewData->PointCloudVertices.resize(PointCount);

	if (bSaveRawPoints)
		NewData->RawPoints.resize(PointCount);

	laszip_U64 PointIndex = 0;
	while (PointIndex < PointCount)
	{
		if (laszip_read_point(LaszipReader))
		{
			LOG.Add("FELASLoader::ReadFile: reading point from laszip reader failed", "FE_LOG_LOADING", FE_LOG_ERROR);
			CloseAndDestroyReader(LaszipReader, true);
			delete NewData;
			return false;
		}

		NewData->PointCloudVertices[PointIndex].X = CurrentPointPointer->X * FileHeader->x_scale_factor + FileHeader->x_offset;
		NewData->PointCloudVertices[PointIndex].Y = CurrentPointPointer->Y * FileHeader->y_scale_factor + FileHeader->y_offset;
		NewData->PointCloudVertices[PointIndex].Z = CurrentPointPointer->Z * FileHeader->z_scale_factor + FileHeader->z_offset;

		NewData->PointCloudVertices[PointIndex].R = unsigned char(CurrentPointPointer->rgb[0] / float(1 << 16) * 255);
		NewData->PointCloudVertices[PointIndex].G = unsigned char(CurrentPointPointer->rgb[1] / float(1 << 16) * 255);
		NewData->PointCloudVertices[PointIndex].B = unsigned char(CurrentPointPointer->rgb[2] / float(1 << 16) * 255);

		if (bSaveRawPoints)
		{
			laszip_point& SavedPoint = NewData->RawPoints[PointIndex];
			SavedPoint = *CurrentPointPointer;
			SavedPoint.extra_bytes = nullptr;
			if (CurrentPointPointer->num_extra_bytes > 0 && CurrentPointPointer->extra_bytes != nullptr)
			{
				SavedPoint.extra_bytes = new laszip_U8[CurrentPointPointer->num_extra_bytes];
				std::memcpy(SavedPoint.extra_bytes, CurrentPointPointer->extra_bytes, CurrentPointPointer->num_extra_bytes);
			}
		}

		PointIndex++;
	}

	CloseAndDestroyReader(LaszipReader, true);
	LastLoadedData = NewData;

	return true;
}

const FELASData* FELASLoader::GetLastLoadedData() const
{
	return LastLoadedData;
}

bool FELASLoader::SaveRawDataToLASOrLAZ(std::vector<FEPointCloudVertex>& RawData, const std::string& FilePath, bool bIsCompressed, double ScaleFactor)
{
	if (RawData.empty())
	{
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: RawData is empty", "FE_LOG_SAVING", FE_LOG_WARNING);
		return false;
	}

	if (FilePath.empty())
	{
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: FilePath is empty", "FE_LOG_SAVING", FE_LOG_WARNING);
		return false;
	}

	if (!IsDLLPresent())
	{
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: LAS/LAZ files are not enabled", "FE_LOG_SAVING", FE_LOG_WARNING);
		return false;
	}

	laszip_POINTER LaszipWriter;
	if (laszip_create(&LaszipWriter))
	{
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Creating laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	laszip_header* FileHeader;
	if (laszip_get_header_pointer(LaszipWriter, &FileHeader))
	{
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Getting header for a file failed", "FE_LOG_SAVING", FE_LOG_ERROR);
		if (laszip_destroy(LaszipWriter))
			LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Destroying laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	// Indicate that we have RGB color data in our point format
	FileHeader->point_data_format = 2;  // Format 2 includes RGB values

	// Update the point data record length to account for RGB data
	// Standard record length is 20 bytes, plus 6 bytes for RGB (2 bytes per color channel)
	FileHeader->point_data_record_length = 26;

	// Set appropriate scale factors for data
	// The LAS format stores coordinates as integers internally,
	// but these integers represent real-world coordinates (typically in meters)
	// Scale factors convert between floating-point and integer representations:
	//   integer_value = real_world_value / scale_factor
	//   real_world_value = integer_value * scale_factor
	//
	// For example, with scale_factor = 0.001:
	// - A real-world coordinate of 10.523 meters becomes integer 10523
	// - An integer value of 8421 represents 8.421 meters in real-world space
	//
	FileHeader->x_scale_factor = ScaleFactor;
	FileHeader->y_scale_factor = ScaleFactor;
	FileHeader->z_scale_factor = ScaleFactor;

	FileHeader->number_of_point_records = static_cast<laszip_U32>(RawData.size());

	if (laszip_open_writer(LaszipWriter, FilePath.c_str(), bIsCompressed))
	{
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Opening laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);

		laszip_CHAR* Error;
		if (laszip_get_error(LaszipWriter, &Error))
			LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Getting laszip error failed", "FE_LOG_SAVING", FE_LOG_ERROR);

		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: " + std::string(Error), "FE_LOG_SAVING", FE_LOG_ERROR);
		if (laszip_destroy(LaszipWriter))
			LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Destroying laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);
		return false;
	}

	laszip_point_struct* CurrentPoint = nullptr;
	CurrentPoint = new laszip_point_struct();
	for (size_t i = 0; i < RawData.size(); i++)
	{
		CurrentPoint->X = static_cast<laszip_I32>(static_cast<double>(RawData[i].X) / FileHeader->x_scale_factor);
		CurrentPoint->Y = static_cast<laszip_I32>(static_cast<double>(RawData[i].Y) / FileHeader->y_scale_factor);
		CurrentPoint->Z = static_cast<laszip_I32>(static_cast<double>(RawData[i].Z) / FileHeader->z_scale_factor);

		// Convert 8-bit colors (0-255) to 16-bit colors (0-65535)
		int CoefficientToConvertTo16Bit = UINT16_MAX / UINT8_MAX;
		CurrentPoint->rgb[0] = static_cast<laszip_U16>(RawData[i].R * CoefficientToConvertTo16Bit);
		CurrentPoint->rgb[1] = static_cast<laszip_U16>(RawData[i].G * CoefficientToConvertTo16Bit);
		CurrentPoint->rgb[2] = static_cast<laszip_U16>(RawData[i].B * CoefficientToConvertTo16Bit);

		if (laszip_set_point(LaszipWriter, CurrentPoint))
		{
			LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Setting point with index " + std::to_string(i) + " failed", "FE_LOG_SAVING", FE_LOG_ERROR);
			delete CurrentPoint;

			if (laszip_close_writer(LaszipWriter))
				LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Closing laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);

			if (laszip_destroy(LaszipWriter))
				LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Destroying laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);

			return false;
		}

		if (laszip_write_point(LaszipWriter))
		{
			LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Writing point with index " + std::to_string(i) + " failed", "FE_LOG_SAVING", FE_LOG_ERROR);
			delete CurrentPoint;

			if (laszip_close_writer(LaszipWriter))
				LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Closing laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);

			if (laszip_destroy(LaszipWriter))
				LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Destroying laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);

			return false;
		}
	}

	if (laszip_close_writer(LaszipWriter))
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Closing laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);

	if (laszip_destroy(LaszipWriter))
		LOG.Add("FELASLoader::SaveRawDataToLASOrLAZ: Destroying laszip writer failed", "FE_LOG_SAVING", FE_LOG_ERROR);

	delete CurrentPoint;
	return true;
}

void FELASLoader::TakeOwnershipOfLastLoadedData(FELASData*& OutData)
{
	OutData = LastLoadedData;
	LastLoadedData = nullptr;
}