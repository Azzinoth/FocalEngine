#include "FEVolumeComponent.h"
using namespace FocalEngine;

FEShader* FEVolumeComponent::GetVolumetricShader() const
{
	return VolumetricShader;
}

void FEVolumeComponent::SetVolumetricShader(FEShader* NewVolumetricShader)
{
	VolumetricShader = NewVolumetricShader;
}

FETexture* FEVolumeComponent::GetVolumetricTexture() const
{
	return VolumetricTexture;
}

void FEVolumeComponent::SetVolumetricTexture(FETexture* NewVolumetricTexture)
{
	if (NewVolumetricTexture == nullptr)
		return;

	if (NewVolumetricTexture->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		return;

	VolumetricTexture = NewVolumetricTexture;
}