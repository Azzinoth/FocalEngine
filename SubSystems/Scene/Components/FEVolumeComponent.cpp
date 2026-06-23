#include "FEVolumeComponent.h"
using namespace FocalEngine;

FENewMaterial* FEVolumeComponent::GetMaterial() const
{
	return VolumeMaterial;
}

#include "Systems/FEVolumeSystem.h"
bool FEVolumeComponent::SetMaterial(FENewMaterial* Material)
{
	if (Material == nullptr)
		return false;

	if (Material->GetMaterialType() != FEMaterialType::Volumetric || Material->GetBlendMode() != FEMaterialBlendMode::Additive)
		return false;

	VolumeMaterial = Material;
	if (Material->GetShader() != nullptr && Material->GetShader()->HasUniform("TransferFunctionTexture"))
	{
		VOLUME_SYSTEM.InitializeTransferFunctionTexture(ParentEntity);
		VOLUME_SYSTEM.BakeTransferFunction(ParentEntity);
	}

	return true;
}