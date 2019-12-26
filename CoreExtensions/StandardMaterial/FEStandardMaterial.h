#pragma once

#include "../../Renderer/FEMaterial.h"

namespace FocalEngine
{
	class FEStandardMaterial : public FEMaterial
	{
	public:
		FEStandardMaterial();
		~FEStandardMaterial();
	private:
		glm::vec3 baseColor;
	};
}