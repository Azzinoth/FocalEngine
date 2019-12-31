#pragma once

#include "../../../Renderer/FEMaterial.h"

namespace FocalEngine
{
	class FESolidColorMaterial : public FEMaterial
	{
	public:
		FESolidColorMaterial();
		~FESolidColorMaterial();
	private:
		glm::vec3 baseColor = glm::vec3(0.0f);
	};
}