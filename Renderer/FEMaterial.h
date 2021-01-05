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

		std::vector<FETexture*> textures;
		std::vector<int> textureBindings;
		std::vector<int> textureChannels;

		void addParameter(FEShaderParam newParameter);
		std::vector<std::string> getParameterList();
		FEShaderParam* getParameter(std::string name);

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

		void setAlbedoMap(FETexture* texture, int subMaterial = 0);
		void setAlbedoMap(int textureIndex, int subMaterial = 0);
		FETexture* getAlbedoMap(int subMaterial = 0);

		void setNormalMap(FETexture* texture, int subMaterial = 0);
		void setNormalMap(int textureIndex, int subMaterial = 0);
		FETexture* getNormalMap(int subMaterial = 0);

		void setAOMap(FETexture* texture, int channel = 0, int subMaterial = 0);
		void setAOMap(int textureIndex, int channel = 0, int subMaterial = 0);
		FETexture* getAOMap(int subMaterial = 0);
		int getAOMapChannel(int subMaterial = 0);

		void setRoughtnessMap(FETexture* texture, int channel = 0, int subMaterial = 0);
		void setRoughtnessMap(int textureIndex, int channel = 0, int subMaterial = 0);
		FETexture* getRoughtnessMap(int subMaterial = 0);
		int getRoughtnessMapChannel(int subMaterial = 0);

		void setMetalnessMap(FETexture* texture, int channel = 0, int subMaterial = 0);
		void setMetalnessMap(int textureIndex, int channel = 0, int subMaterial = 0);
		FETexture* getMetalnessMap(int subMaterial = 0);
		int getMetalnessMapChannel(int subMaterial = 0);

		void setDisplacementMap(FETexture* texture, int channel = 0, int subMaterial = 0);
		void setDisplacementMap(int textureIndex, int channel = 0, int subMaterial = 0);
		FETexture* getDisplacementMap(int subMaterial = 0);
		int getDisplacementMapChannel(int subMaterial = 0);

		bool addTexture(FETexture* texture);
		void removeTexture(FETexture* texture);
		void removeTexture(int textureIndex);
		void clearAllTexturesInfo();
	private:
		glm::vec3 diffuseColor;
		glm::vec3 baseColor;

		int placeTextureInList(FETexture* texture);

		const int albedoBindingIndex = 0;
		const int normalBindingIndex = 1;
		const int AOBindingIndex = 2;
		const int roughtnessBindingIndex = 3;
		const int metalnessBindingIndex = 4;
		const int displacementBindingIndex = 5;

		float normalMapIntensity = 1.0f;
		float metalness = 0.01f;
		float metalnessMapIntensity = 1.0f;
		float roughtness = 0.8f;
		float roughtnessMapIntensity = 1.0f;
		float ambientOcclusionMapIntensity = 1.0f;
		float ambientOcclusionIntensity = 1.0f;
	};
}

#endif
