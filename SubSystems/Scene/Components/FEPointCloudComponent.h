#pragma once
#include "../Renderer/FEGameModel.h"

namespace FocalEngine
{
	struct FEPointCloudComponent
	{
		FEPointCloudComponent() {};
		//FEPointcloudComponent(FEGameModel* GameModelToSet);
		FEPointCloudComponent(const FEPointCloudComponent& Other);
		void operator=(const FEPointCloudComponent& Other);

		// Maybe that part should be abstracted.
		bool IsVisible() const;
		void SetVisibility(bool NewValue);
	private:

		bool bVisible = true;
	};
}