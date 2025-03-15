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
	private:

		FEPointCloud* PointCloud = nullptr;
		bool bVisible = true;
	};
}