#pragma once
#include "../ResourceManager/BaseResources/FEPointCloud.h"

namespace FocalEngine
{
	struct FEPointCloudComponent
	{
		FEPointCloudComponent() {};
		FEPointCloudComponent(FEPointCloud* PointCloudToSet);
		FEPointCloudComponent(const FEPointCloudComponent& Other);
		void operator=(const FEPointCloudComponent& Other);

		FEPointCloud* GetPointCloud() const;
		void SetPointCloud(FEPointCloud* NewPointCloud);

		glm::vec3 GetGlobalColorOverride() const;
		void SetGlobalColorOverride(const glm::vec3& NewColor);

		bool IsUsingGlobalColorOverride() const;
		void SetUseGlobalColorOverride(bool NewValue);
	private:
		FEPointCloud* PointCloud = nullptr;

		glm::vec3 GlobalColorOverride;
		bool bUseGlobalColorOverride = false;
	};
}