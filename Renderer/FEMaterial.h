#pragma once

#ifndef FEMATERIAL_H
#define FEMATERIAL_H

#include "FEShader.h"

namespace FocalEngine
{
	class FERenderer;
	class FEResourceManager;

	class FEMaterial : public FEObject
	{
		friend FERenderer;
		friend FEResourceManager;
	public:
		FEMaterial(std::string Name);
		~FEMaterial();

		FEShader* Shader = nullptr;
		
		void Bind();
		void UnBind();

		std::vector<FETexture*> Textures;
		std::vector<int> TextureBindings;
		std::vector<int> TextureChannels;

		std::vector<std::string> GetUniformNameList() const;
		std::vector<std::string> GetUniformVariationsNameList() const;

		bool SetUniformVariation(FEShaderUniformValue NewUniformVariation);
		FEShaderUniformValue* GetUniformVariation(std::string Name);

		glm::vec3 GetBaseColor();
		void SetBaseColor(glm::vec3 NewValue);

		float GetMetalness() const;
		void SetMetalness(float NewValue);

		float GetRoughness() const;
		void SetRoughness(float NewValue);

		float GetRoughnessMapIntensity() const;
		void SetRoughnessMapIntensity(float NewValue);

		float GetMetalnessMapIntensity() const;
		void SetMetalnessMapIntensity(float NewValue);

		float GetNormalMapIntensity() const;
		void SetNormalMapIntensity(float NewValue);

		float GetAmbientOcclusionIntensity() const;
		void SetAmbientOcclusionIntensity(float NewValue);

		float GetAmbientOcclusionMapIntensity() const;
		void SetAmbientOcclusionMapIntensity(float NewValue);

		float GetDisplacementMapIntensity() const;
		void SetDisplacementMapIntensity(float NewValue);

		void SetAlbedoMap(FETexture* Texture, int SubMaterial = 0);
		void SetAlbedoMap(int TextureIndex, int SubMaterial = 0);
		FETexture* GetAlbedoMap(int SubMaterial = 0);

		void SetNormalMap(FETexture* Texture, int SubMaterial = 0);
		void SetNormalMap(int TextureIndex, int SubMaterial = 0);
		FETexture* GetNormalMap(int SubMaterial = 0);

		void SetAOMap(FETexture* Texture, int Channel = 0, int SubMaterial = 0);
		void SetAOMap(int TextureIndex, int Channel = 0, int SubMaterial = 0);
		FETexture* GetAOMap(int SubMaterial = 0);
		int GetAOMapChannel(int SubMaterial = 0) const;

		void SetRoughnessMap(FETexture* Texture, int Channel = 0, int SubMaterial = 0);
		void SetRoughnessMap(int TextureIndex, int Channel = 0, int SubMaterial = 0);
		FETexture* GetRoughnessMap(int SubMaterial = 0);
		int GetRoughnessMapChannel(int SubMaterial = 0) const;

		void SetMetalnessMap(FETexture* Texture, int Channel = 0, int SubMaterial = 0);
		void SetMetalnessMap(int TextureIndex, int Channel = 0, int SubMaterial = 0);
		FETexture* GetMetalnessMap(int SubMaterial = 0);
		int GetMetalnessMapChannel(int SubMaterial = 0) const;

		void SetDisplacementMap(FETexture* Texture, int Channel = 0, int SubMaterial = 0);
		void SetDisplacementMap(int TextureIndex, int Channel = 0, int SubMaterial = 0);
		FETexture* GetDisplacementMap(int SubMaterial = 0);
		int GetDisplacementMapChannel(int SubMaterial = 0) const;

		bool AddTexture(FETexture* Texture);
		bool IsTextureInList(const FETexture* Texture) const;
		void RemoveTexture(FETexture* Texture);
		void RemoveTexture(int TextureIndex);
		void ClearAllTexturesInfo();
		int GetUsedTexturesCount() const;

		bool IsCompackPacking();
		void SetCompackPacking(bool NewValue);

		bool IsCompackPackingPossible();

		float GetTiling() const;
		void SetTiling(float NewValue);
	private:
		std::unordered_map<std::string, FEShaderUniformValue> UniformVariations;

		int PlaceTextureInList(FETexture* Texture);
		void SetTextureBinding(int Index, int TextureIndex, int SubMaterial, int Channel = -2);
		void ClearTextureBinding(int Index, int SubMaterial, int Channel = -2);

		const int AlbedoBindingIndex = 0;
		const int NormalBindingIndex = 1;
		const int AOBindingIndex = 2;
		const int RoughnessBindingIndex = 3;
		const int MetalnessBindingIndex = 4;
		const int DisplacementBindingIndex = 5;

		float NormalMapIntensity = 1.0f;
		float Metalness = 0.01f;
		float MetalnessMapIntensity = 1.0f;
		float Roughness = 0.8f;
		float RoughnessMapIntensity = 1.0f;
		float AmbientOcclusionMapIntensity = 1.0f;
		float AmbientOcclusionIntensity = 1.0f;
		float DisplacementMapIntensity = 1.0f;
		float Tiling = 1.0f;
		bool bCompackPacking = false;

		FETexture* GetSpecifiedMap(int BindingIndex, int SubMaterial = 0);
	};
}

#endif
