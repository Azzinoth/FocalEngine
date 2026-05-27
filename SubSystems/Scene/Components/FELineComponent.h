#pragma once
#include "../ResourceManager/BaseResources/FELineCollection.h"

namespace FocalEngine
{
	struct FELineComponent
	{
		FELineComponent() {};
		FELineComponent(FELineCollection* LineCollectionToSet);

		FELineCollection* GetLineCollection() const;
		void SetLineCollection(FELineCollection* NewLineCollection);
	private:
		FELineCollection* LineCollection = nullptr;
	};
}