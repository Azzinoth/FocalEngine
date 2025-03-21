#pragma once
#include "../Renderer/FEPointCloud.h"

namespace FocalEngine
{
	struct FEPointCloudComponent
	{
		FEPointCloudComponent() {};
		FEPointCloudComponent(FEPointCloud* PointCloudToSet);
		FEPointCloudComponent(const FEPointCloudComponent& Other);
		void operator=(const FEPointCloudComponent& Other);

		// Maybe that part should be abstracted.
		bool IsVisible() const;
		void SetVisibility(bool NewValue);

		FEPointCloud* GetPointCloud() const;
		void SetPointCloud(FEPointCloud* NewPointCloud);

		glm::vec3 GetGlobalColorOverride() const;
		void SetGlobalColorOverride(const glm::vec3& NewColor);

		bool IsUsingGlobalColorOverride() const;
		void SetUseGlobalColorOverride(bool NewValue);
	private:
		FEPointCloud* PointCloud = nullptr;
		bool bVisible = true;

		glm::vec3 FEGlobalColorOverride;
		bool bUseGlobalColorOverride = false;
	};
}