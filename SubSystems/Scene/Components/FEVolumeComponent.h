#pragma once
#include "../Core/FEObject.h"
#include "../ResourceManager/BaseResources/FETexture.h"

namespace FocalEngine
{
	struct FEVolumeComponent 
	{
		friend class FERenderer;
		friend class FEVolumeSystem;

		FEVolumeComponent() {};

		FEShader* GetVolumetricShader() const;
		void SetVolumetricShader(FEShader* NewVolumetricShader);

		FETexture* GetVolumetricTexture() const;
		void SetVolumetricTexture(FETexture* NewVolumetricTexture);

	private:
		// FE_TO_DO: Implement volume material ?
		FEShader* VolumetricShader = nullptr;
		FETexture* VolumetricTexture = nullptr;
	};
}