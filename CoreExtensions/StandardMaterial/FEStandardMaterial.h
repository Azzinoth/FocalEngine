#pragma once

#include "../../Renderer/FEMaterial.h"

namespace FocalEngine
{
	class FEStandardMaterial : public FEMaterial
	{
	public:
		FEStandardMaterial();
		~FEStandardMaterial();

		glm::vec3 getBaseColor();
		void setBaseColor(glm::vec3 newColor);
	private:
		glm::vec3 baseColor;
	};
}