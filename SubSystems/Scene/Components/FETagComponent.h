#pragma once
#include "../Core/FEObject.h"

namespace FocalEngine
{
	struct FETagComponent
	{
		FETagComponent() {};
		FETagComponent(std::string TagToSet);

		std::string GetTag() const;
		void SetTag(std::string NewTag);
	private:
		std::string Tag = "";
	};
}