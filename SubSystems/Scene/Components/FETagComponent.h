#pragma once
#include "../Core/FEObject.h"

namespace FocalEngine
{
	struct FETagComponent
	{
		std::string Tag = "";
		FETagComponent() {};
		FETagComponent(std::string TagToSet);
	};
}