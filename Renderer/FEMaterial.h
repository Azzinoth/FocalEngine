#pragma once

#ifndef FEMATERIAL_H
#define FEMATERIAL_H

#include "FEShader.h"
#include "../CoreExtensions/StandardMaterial/FEStandardShader.h"

#define MAX_TEXTURE_COUNT_FOR_MATERIAL = 10;

namespace FocalEngine
{
	class FERenderer;

	class FEMaterial
	{
		friend FERenderer;
	public:
		FEMaterial();
		~FEMaterial();

		std::vector<FEShader*> shaders;
		std::vector<FETexture*> textures;

		void bind();
		void unBind();
	private:
		glm::vec3 diffuseColor;
	};
}

#endif
