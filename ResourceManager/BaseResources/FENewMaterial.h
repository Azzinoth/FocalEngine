#pragma once

#ifndef FENEWMATERIAL_H
#define FENEWMATERIAL_H

#include "FEShader.h"

namespace FocalEngine
{
	class FERenderer;
	class FEResourceManager;

	enum class FEMaterialType
	{
		Surface,
		Volumetric,
		PostProcess,
		UI
	};

	enum class FEMaterialBlendMode
	{
		Opaque,
		Masked,
		Translucent,
		Additive
	};

	class FENewMaterial : public FEObject
	{
		friend FERenderer;
		friend FEResourceManager;
	public:
		FENewMaterial(std::string Name);
		~FENewMaterial();

		Json::Value ToJSON();
		void FromJSON(const Json::Value& MaterialData);

		FEMaterialType GetMaterialType() const;
		void SetMaterialType(FEMaterialType NewMaterialType);

		FEMaterialBlendMode GetBlendMode() const;
		void SetBlendMode(FEMaterialBlendMode NewBlendMode);

		FEShader* GetShader() const;
		bool SetShader(FEShader* NewShader);
		
		void Bind();
		void UnBind();

		bool HasUniform(std::string UniformName) const;
		std::vector<std::string> GetUniformNameList() const;
		std::vector<std::string> GetUniformOverrideNameList() const;

		bool SetUniformOverride(FEShaderUniformValue NewUniformOverride);
		
		template<typename T>
		bool UpdateUniformOverrideData(const std::string& UniformName, const T& Value);
		FEShaderUniformValue* GetUniformOverride(const std::string& Name);

		bool SetTextureOverride(const std::string& UniformName, const std::string& TextureID);
		FETexture* GetTextureOverride(const std::string& UniformName) const;

		bool IsUserTextureBound(const std::string& TextureID) const;
		int GetUsedTexturesCount() const;
		bool IsAllUsedTexturesNonNullptrs() const;
	private:
		FEMaterialType MaterialType = FEMaterialType::Surface;
		FEMaterialBlendMode BlendMode = FEMaterialBlendMode::Opaque;

		std::unordered_map<std::string, FEShaderUniformValue> UniformOverrides;
		std::unordered_map<std::string, FETexture*> TextureOverrides;

		FEShader* Shader = nullptr;

		bool TryToUpdateEngineProvidedUniform(FEShaderUniform* CurrentUniform);
	};

#include "FENewMaterial.inl"
}

#endif