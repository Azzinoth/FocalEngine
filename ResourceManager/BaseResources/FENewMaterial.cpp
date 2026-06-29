#include "FENewMaterial.h"
#include <algorithm>
using namespace FocalEngine;

FENewMaterial::FENewMaterial(const std::string Name) : FEObject(FE_NEW_MATERIAL, Name)
{
	this->Name = Name;
}

FENewMaterial::~FENewMaterial()
{
}

FEShader* FENewMaterial::GetShader() const
{
	return Shader;
}

FEMaterialType FENewMaterial::GetMaterialType() const
{
	return MaterialType;
}

void FENewMaterial::SetMaterialType(FEMaterialType NewMaterialType)
{
	MaterialType = NewMaterialType;
}

FEMaterialBlendMode FENewMaterial::GetBlendMode() const
{
	return BlendMode;
}

void FENewMaterial::SetBlendMode(FEMaterialBlendMode NewBlendMode)
{
	BlendMode = NewBlendMode;
}

bool FENewMaterial::SetShader(FEShader* NewShader)
{
	UniformOverrides.clear();
	TextureOverrides.clear();
	Shader = NewShader;

	if (NewShader != nullptr)
	{
		auto UniformIterator = Shader->Uniforms.begin();
		while (UniformIterator != Shader->Uniforms.end())
		{
			if (UniformIterator->second.IsProvidedByEngine())
			{
				UniformIterator++;
				continue;
			}

			if (FEShaderUniform::IsTextureType(UniformIterator->second.GetType()))
			{
				TextureOverrides[UniformIterator->first] = nullptr;
			}
			else
			{
				UniformOverrides[UniformIterator->first] = FEShaderUniformValue(UniformIterator->second.CurrentValue);
			}

			UniformIterator++;
		}
	}

	return true;
}

#include "../FEResourceManager.h"
bool FENewMaterial::SetTextureOverride(const std::string& UniformName, const std::string& TextureID)
{
	if (Shader == nullptr)
		return false;

	FEShaderUniform* Uniform = Shader->GetUniform(UniformName);
	if (Uniform == nullptr)
	{
		LOG.Add("FENewMaterial::SetTextureOverride() failed to set texture on non existing uniform", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return false;
	}

	if (!FEShaderUniform::IsTextureType(Uniform->GetType()))
	{
		LOG.Add("FENewMaterial::SetTextureOverride() uniform " + UniformName + " is not a texture type", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return false;
	}

	FETexture* TextureToAdd = RESOURCE_MANAGER.GetTexture(TextureID);
	if (TextureToAdd == nullptr)
	{
		LOG.Add("FENewMaterial::SetTextureOverride() failed to find texture with ID " + TextureID, "FE_LOG_RENDERING", FE_LOG_WARNING);
		return false;
	}

	TextureOverrides[UniformName] = TextureToAdd;
	return true;
}

FETexture* FENewMaterial::GetTextureOverride(const std::string& UniformName) const
{
	auto TextureBindingIterator = TextureOverrides.find(UniformName);
	if (TextureBindingIterator == TextureOverrides.end())
		return nullptr;

	return TextureBindingIterator->second;
}

void FENewMaterial::Bind()
{
	if (Shader == nullptr)
		return;

	Shader->Start();

	// First will take care of uniforms that are provided by the engine.
	auto AllShaderUniformIterator = Shader->Uniforms.begin();
	while (AllShaderUniformIterator != Shader->Uniforms.end())
	{
		if (AllShaderUniformIterator->second.IsProvidedByEngine())
			TryToUpdateEngineProvidedUniform(&AllShaderUniformIterator->second);

		AllShaderUniformIterator++;
	}

	// Then will take care of texture bindings.
	auto TextureBindingIterator = TextureOverrides.begin();
	while (TextureBindingIterator != TextureOverrides.end())
	{
		FEShaderUniform* Uniform = Shader->GetUniform(TextureBindingIterator->first);
		FETexture* TextureToBind = TextureBindingIterator->second;
		if (TextureToBind != nullptr)
		{
			int TextureUnit = Uniform->GetValue<int>();
			TextureToBind->Bind(TextureUnit);
		}

		TextureBindingIterator++;
	}

	int IterationCount = 0;
	auto UniformOverridesIterator = UniformOverrides.begin();
	while (UniformOverridesIterator != UniformOverrides.end())
	{
		FEShaderUniform* Uniform = Shader->GetUniform(UniformOverridesIterator->first);
		if (Uniform == nullptr)
		{
			LOG.Add("FENewMaterial::Bind() failed to set data on non existing uniform: " + UniformOverridesIterator->first, "FE_LOG_RENDERING", FE_LOG_WARNING);
			UniformOverridesIterator++;
			continue;
		}

		Uniform->CurrentValue = UniformOverridesIterator->second;
		UniformOverridesIterator++;
		IterationCount++;
	}

	Shader->LoadUniformsDataToGPU();
}

#include "../../Renderer/FERenderer.h"
bool FENewMaterial::TryToUpdateEngineProvidedUniform(FEShaderUniform* CurrentUniform)
{
	if (FEShaderUniform::IsTextureType(CurrentUniform->GetType()))
		return RENDERER.BindEngineProvidedTexture(Shader, CurrentUniform->GetName());

	const EntityBasedEngineProvidedData& EntityBasedData = RENDERER.GetCurrentEntityBasedEngineProvidedData();
	const CameraBasedEngineProvidedData& CameraBasedData = RENDERER.GetCurrentCameraBasedEngineProvidedData();
	const std::string& Name = CurrentUniform->GetName();

	bool bResult = false;
	if (Name == "FEWorldMatrix")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::mat4>(EntityBasedData.WorldMatrix);
	}
	else if (Name == "FEViewMatrix")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::mat4>(CameraBasedData.ViewMatrix);
	}
	else if (Name == "FEInverseViewMatrix")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::mat4>(CameraBasedData.InverseViewMatrix);
	}
	else if (Name == "FEProjectionMatrix")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::mat4>(CameraBasedData.ProjectionMatrix);
	}
	else if (Name == "FEInverseProjectionMatrix")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::mat4>(CameraBasedData.InverseProjectionMatrix);
	}
	else if (Name == "FEPVMMatrix")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::mat4>(CameraBasedData.ProjectionMatrix * CameraBasedData.ViewMatrix * EntityBasedData.WorldMatrix);
	}
	else if (Name == "FECameraPosition")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::vec3>(CameraBasedData.CameraPosition);
	}
	else if (Name == "FECameraDirection")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<glm::vec3>(CameraBasedData.CameraDirection);
	}
	else if (Name == "FENearPlane")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<float>(CameraBasedData.NearPlane);
	}
	else if (Name == "FEFarPlane")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<float>(CameraBasedData.FarPlane);
	}
	else if (Name == "FEGamma")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<float>(CameraBasedData.Gamma);
	}
	else if (Name == "FEExposure")
	{
		bResult = CurrentUniform->CurrentValue.SetValue<float>(CameraBasedData.Exposure);
	}
	else
	{
		LOG.Add("FENewMaterial::TryToUpdateEngineProvidedUniform() no provider for engine uniform: " + Name, "FE_LOG_RENDERING", FE_LOG_WARNING);
	}

	return bResult;
}

void FENewMaterial::UnBind()
{
	if (Shader != nullptr)
		Shader->Stop();
}

bool FENewMaterial::SetUniformOverride(const FEShaderUniformValue NewUniformOverride)
{
	if (Shader->GetUniform(NewUniformOverride.GetName()) == nullptr)
	{
		LOG.Add("FENewMaterial::SetUniformOverride() failed to set override on non existing uniform", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return false;
	}

	UniformOverrides[NewUniformOverride.GetName()] = NewUniformOverride;
	return true;
}

bool FENewMaterial::HasUniform(std::string UniformName) const
{
	return Shader->HasUniform(UniformName);
}

std::vector<std::string> FENewMaterial::GetUniformNameList() const
{
	return Shader->GetUniformNameList();
}

std::vector<std::string> FENewMaterial::GetUniformOverrideNameList() const
{
	FE_MAP_TO_STR_VECTOR(UniformOverrides);
}

FEShaderUniformValue* FENewMaterial::GetUniformOverride(const std::string& Name)
{
	if (UniformOverrides.find(Name) == UniformOverrides.end())
		return nullptr;

	return &UniformOverrides[Name];
}

bool FENewMaterial::IsUserTextureBound(const std::string& TextureID) const
{
	if (TextureID.empty())
		return false;

	for (const auto& TextureBinding : TextureOverrides)
	{
		const FETexture* BoundTexture = TextureBinding.second;
		if (BoundTexture != nullptr && BoundTexture->GetObjectID() == TextureID)
			return true;
	}

	return false;
}

int FENewMaterial::GetUsedTexturesCount() const
{
	int Result = 0;
	for (const auto& TextureBinding : TextureOverrides)
	{
		if (TextureBinding.second != nullptr)
			Result++;
	}

	return Result;
}

bool FENewMaterial::IsAllUsedTexturesNonNullptrs() const
{
	for (const auto& TextureBinding : TextureOverrides)
	{
		if (TextureBinding.second == nullptr)
			return false;
	}

	return true;
}

std::vector<std::pair<std::string, FETexture*>> FENewMaterial::GetAllTextureOverridePair() const
{
	std::vector<std::pair<std::string, FETexture*>> Result;
	for (const auto& TextureBinding : TextureOverrides)
		Result.push_back(TextureBinding);
	
	return Result;
}