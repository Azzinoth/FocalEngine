#pragma once

#ifndef FEMATERIAL_H
#define FEMATERIAL_H

#include "FEShader.h"
#include "../CoreExtensions/StandardMaterial/SolidColorMaterial/FESolidColorShader.h"
#include "../CoreExtensions/StandardMaterial/PhongMaterial/FEPhongShader.h"

#define MAX_TEXTURE_COUNT_FOR_MATERIAL = 10;

namespace FocalEngine
{
	class FERenderer;
	class FEResourceManager;

	class FEMaterial
	{
		friend FERenderer;
		friend FEResourceManager;
	public:
		FEMaterial();
		~FEMaterial();

		FEShader* shader;
		
		virtual void bind();
		virtual void unBind();

		void setParam(std::string name, int newData);
		void setParam(std::string name, float newData);
		void setParam(std::string name, glm::vec2 newData);
		void setParam(std::string name, glm::vec3 newData);
		void setParam(std::string name, glm::vec4 newData);
		void setParam(std::string name, glm::mat4 newData);

		FETexture* albedoMap;
		FETexture* normalMap;
		FETexture* roughtnessMap;
		FETexture* metalnessMap;
		FETexture* AOMap;
		FETexture* displacementMap;

		void addParameter(FEShaderParam newParameter);
		std::vector<std::string> getParameterList();
		FEShaderParam* getParameter(std::string name);

		std::string getName();
	private:
		std::string name;
		glm::vec3 diffuseColor;
		void setName(std::string newName);
	};
}

#endif
