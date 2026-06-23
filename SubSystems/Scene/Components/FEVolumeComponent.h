#pragma once
#include "../Core/FEObject.h"
#include "../ResourceManager/BaseResources/FENewMaterial.h"

namespace FocalEngine
{
	class FEEntity;

	struct FEVolumeComponent
	{
		friend class FERenderer;
		friend class FEVolumeSystem;

		FEVolumeComponent() {};

		FENewMaterial* GetMaterial() const;
		bool SetMaterial(FENewMaterial* Material);
	private:
		FENewMaterial* VolumeMaterial = nullptr;
		FEEntity* ParentEntity = nullptr;
	};
}