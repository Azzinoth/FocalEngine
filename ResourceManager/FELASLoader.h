#pragma once

#include "../SubSystems/FileSystem/FEFileSystem.h"
#include "../Core/Geometry/FEGeometry.h"
#include "BaseResources/FEPointCloud.h"
#include "../ThirdParty/laszip/laszip_api.h"

namespace FocalEngine
{
	struct FELASData
	{
		friend class FELASLoader;

		FELASData() = default;
		FELASData(const FELASData&) = delete;
		FELASData& operator=(const FELASData&) = delete;
		~FELASData();

		std::string FilePath;
		bool bWasCompressed = false;

		laszip_header* Header = nullptr;
		std::vector<laszip_point> RawPoints;

		glm::dvec3 AppliedShift = glm::dvec3(0.0);
		std::vector<FEPointCloudVertexDouble> PointCloudVertices;

	private:
		void FreeHeaderCopy();
	};

	class FELASLoader
	{
		friend class FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FELASLoader)

		bool IsDLLPresent();

		bool ReadFile(const std::string& FilePath, bool bSaveHeaderCopy = false, bool bSaveRawPoints = false);
		bool SaveRawDataToLASOrLAZ(std::vector<FEPointCloudVertex>& RawData, const std::string& FilePath, bool bIsCompressed = true, double ScaleFactor = 0.001);

		const FELASData* GetLastLoadedData() const;
		// Transfers ownership of the last loaded data to the caller.
		void TakeOwnershipOfLastLoadedData(FELASData*& OutData);
	private:
		SINGLETON_PRIVATE_PART(FELASLoader)

		bool bDLLPresent = false;
		FELASData* LastLoadedData = nullptr;

		void ClearLoadedData();
		static laszip_header* DeepCopyHeader(const laszip_header* Source);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetLASLoader();
#define LAS_LOADER (*static_cast<FELASLoader*>(GetLASLoader()))
#else
#define LAS_LOADER FELASLoader::GetInstance()
#endif
}