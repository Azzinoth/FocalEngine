#pragma once

#ifndef FEMATERIAL_H
#define FEMATERIAL_H

#include "FEShader.h"

namespace FocalEngine
{
	class FERenderer;
	class FEResourceManager;

	class FEMaterial : public FEAsset
	{
		friend FERenderer;
		friend FEResourceManager;
	public:
		FEMaterial(std::string Name);
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
		FETexture* MRAOMap;

		void addParameter(FEShaderParam newParameter);
		std::vector<std::string> getParameterList();
		FEShaderParam* getParameter(std::string name);

		std::string getName();

		glm::vec3 getBaseColor();
		// Only infuence color of object if shader with such uniform is applied.
		void setBaseColor(glm::vec3 newBaseColor);

		float getMetalness();
		void setMetalness(float newMetalness);

		float getRoughtness();
		void setRoughtness(float newRoughtness);

		float getRoughtnessMapIntensity();
		void setRoughtnessMapIntensity(float newRoughtnessMapIntensity);

		float getMetalnessMapIntensity();
		void setMetalnessMapIntensity(float newMetalnessMapIntensity);

		float getNormalMapIntensity();
		void setNormalMapIntensity(float newNormalMapIntensity);

		float getAmbientOcclusionIntensity();
		void setAmbientOcclusionIntensity(float newAmbientOcclusionIntensity);

		float getAmbientOcclusionMapIntensity();
		void setAmbientOcclusionMapIntensity(float newAmbientOcclusionMapIntensity);
	private:
		std::string name;
		glm::vec3 diffuseColor;
		void setName(std::string newName);
		glm::vec3 baseColor;

		float metalness = 0.01f;
		float metalnessMapIntensity = 1.0f;
		float roughtness = 0.8f;
		float roughtnessMapIntensity = 1.0f;
		float normalMapIntensity = 1.0f;
		float ambientOcclusionMapIntensity = 1.0f;
		float ambientOcclusionIntensity = 1.0f;
	};
}

#endif
