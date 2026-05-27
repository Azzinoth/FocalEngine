#include "FEPointCloudComponent.h"
using namespace FocalEngine;

FEPointCloudComponent::FEPointCloudComponent(FEPointCloud* PointCloudToSet)
{
	PointCloud = PointCloudToSet;
}

FEPointCloudComponent::FEPointCloudComponent(const FEPointCloudComponent& Other)
{
	PointCloud = Other.PointCloud;
	GlobalColorOverride = Other.GlobalColorOverride;
	bUseGlobalColorOverride = Other.bUseGlobalColorOverride;
}

void FEPointCloudComponent::operator=(const FEPointCloudComponent& Other)
{
	PointCloud = Other.PointCloud;
	GlobalColorOverride = Other.GlobalColorOverride;
	bUseGlobalColorOverride = Other.bUseGlobalColorOverride;
}

FEPointCloudComponent::~FEPointCloudComponent()
{

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