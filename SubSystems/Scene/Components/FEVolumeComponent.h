#pragma once
#include "../Core/FEObject.h"

namespace FocalEngine
{
	struct FEVolumeComponent 
	{
		FEVolumeComponent() {};
		//FEVolumeComponent(FELineCollection* LineCollectionToSet);

		//FELineCollection* GetLineCollection() const;
		//void SetLineCollection(FELineCollection* NewLineCollection);
	private:
		void* DummyData = nullptr;

	};
}