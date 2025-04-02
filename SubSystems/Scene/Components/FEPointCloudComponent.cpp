#include "FEPointCloudComponent.h"
using namespace FocalEngine;

FEPointCloudComponent::FEPointCloudComponent(FEPointCloud* PointCloudToSet)
{
	PointCloud = PointCloudToSet;
}

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

FEPointCloud* FEPointCloudComponent::GetPointCloud() const
{
	return PointCloud;
}

void FEPointCloudComponent::SetPointCloud(FEPointCloud* NewPointCloud)
{
	PointCloud = NewPointCloud;
}

glm::vec3 FEPointCloudComponent::GetGlobalColorOverride() const
{
	return GlobalColorOverride;
}

void FEPointCloudComponent::SetGlobalColorOverride(const glm::vec3& NewColor)
{
	GlobalColorOverride = NewColor;
}

bool FEPointCloudComponent::IsUsingGlobalColorOverride() const
{
	return bUseGlobalColorOverride;
}

void FEPointCloudComponent::SetUseGlobalColorOverride(bool NewValue)
{
	bUseGlobalColorOverride = NewValue;
}