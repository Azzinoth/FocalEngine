#include "FEPointCloudComponent.h"
using namespace FocalEngine;

//FEPointcloudComponent::FEPointcloudComponent(FEGameModel* GameModelToSet)
//{
//	GameModel = GameModelToSet;
//}

FEPointCloudComponent::FEPointCloudComponent(const FEPointCloudComponent& Other)
{
	bVisible = Other.bVisible;
}

void FEPointCloudComponent::operator=(const FEPointCloudComponent& Other)
{
	bVisible = Other.bVisible;
}

bool FEPointCloudComponent::IsVisible() const
{
	return bVisible;
}

void FEPointCloudComponent::SetVisibility(bool NewValue)
{
	bVisible = NewValue;
}