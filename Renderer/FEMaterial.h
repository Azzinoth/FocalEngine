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

		virtual void bind();
		virtual void unBind();

		void setParam(std::string name, int newData);
		void setParam(std::string name, float newData);
		void setParam(std::string name, glm::vec2 newData);
		void setParam(std::string name, glm::vec3 newData);
		void setParam(std::string name, glm::vec4 newData);
		void setParam(std::string name, glm::mat4 newData);
	private:
		glm::vec3 diffuseColor;
	};
}

#endif
