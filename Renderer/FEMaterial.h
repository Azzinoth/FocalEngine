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

		void addTexture(FETexture* newTexture);

		short getID();

		bool addParameter(FEShaderParam newParameter);
		std::vector<std::string> getParameterList();
		FEShaderParam* getParametr(std::string name);

		std::string getName();
		void setName(std::string newName);
	private:
		short id = -1;
		std::string name;

		std::vector<FETexture*> textures;
		glm::vec3 diffuseColor;
	};
}

#endif
