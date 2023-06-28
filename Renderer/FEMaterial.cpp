#include "FEMaterial.h"
#include <algorithm>
using namespace FocalEngine;

FEMaterial::FEMaterial(const std::string Name) : FEObject(FE_MATERIAL, Name)
{
	this->Name = Name;

	Textures.resize(FE_MAX_TEXTURES_PER_MATERIAL);
	std::for_each(Textures.begin(), Textures.end(), [](FETexture*& Item) {
		Item = nullptr;
	});

	TextureBindings.resize(FE_MAX_TEXTURES_PER_MATERIAL);
	std::for_each(TextureBindings.begin(), TextureBindings.end(), [](int& Item) {
		Item = -1;
	});

	TextureChannels.resize(FE_MAX_TEXTURES_PER_MATERIAL);
	std::for_each(TextureChannels.begin(), TextureChannels.end(), [](int& Item) {
		Item = -1;
	});
}

FEMaterial::~FEMaterial()
{
}

void FEMaterial::Bind()
{
	if (Shader != nullptr)
		Shader->Start();

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (Textures[i] != nullptr)
			Textures[i]->Bind(static_cast<int>(i));
	}

	// #fix such specific if statement in this class is not clean coding
	Shader->UpdateParameterData("baseColor", BaseColor);
}

void FEMaterial::UnBind()
{
	if (Shader != nullptr)
		Shader->Stop();

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (Textures[i] != nullptr)
			Textures[i]->UnBind();
	}
}

void FEMaterial::SetParam(const std::string Name, const int NewData) const
{
	Shader->UpdateParameterData(Name, NewData);
}

void FEMaterial::SetParam(const std::string Name, const float NewData) const
{
	Shader->UpdateParameterData(Name, NewData);
}

void FEMaterial::SetParam(const std::string Name, const glm::vec2 NewData) const
{
	Shader->UpdateParameterData(Name, NewData);
}

void FEMaterial::SetParam(const std::string Name, const glm::vec3 NewData) const
{
	Shader->UpdateParameterData(Name, NewData);
}

void FEMaterial::SetParam(const std::string Name, const glm::vec4 NewData) const
{
	Shader->UpdateParameterData(Name, NewData);
}

void FEMaterial::SetParam(const std::string Name, const glm::mat4 NewData) const
{
	Shader->UpdateParameterData(Name, NewData);
}

void FEMaterial::AddParameter(const FEShaderParam NewParameter) const
{
	Shader->AddParameter(NewParameter);
}

std::vector<std::string> FEMaterial::GetParameterList() const
{
	return Shader->GetParameterList();
}

FEShaderParam* FEMaterial::GetParameter(const std::string Name) const
{
	return Shader->GetParameter(Name);
}

glm::vec3 FEMaterial::GetBaseColor() const
{
	return BaseColor;
}

// Only influence color of object if shader with such uniform is applied.
void FEMaterial::SetBaseColor(const glm::vec3 NewValue)
{
	BaseColor = NewValue;
}

float FEMaterial::GetMetalness() const
{
	return Metalness;
}

void FEMaterial::SetMetalness(float NewValue)
{
	if (NewValue > 1.0f)
		NewValue = 1.0f;

	if (NewValue < 0.0f)
		NewValue = 0.0f;

	Metalness = NewValue;
}

float FEMaterial::GetRoughness() const
{
	return Roughness;
}

void FEMaterial::SetRoughness(float NewValue)
{
	if (NewValue > 1.0f)
		NewValue = 1.0f;

	if (NewValue < 0.0f)
		NewValue = 0.0f;

	Roughness = NewValue;
}

float FEMaterial::GetNormalMapIntensity() const
{
	return NormalMapIntensity;
}

void FEMaterial::SetNormalMapIntensity(float NewValue)
{
	if (NewValue > 1.0f)
		NewValue = 1.0f;

	if (NewValue < 0.0f)
		NewValue = 0.0f;

	NormalMapIntensity = NewValue;
}

float FEMaterial::GetAmbientOcclusionIntensity() const
{
	return AmbientOcclusionIntensity;
}

void FEMaterial::SetAmbientOcclusionIntensity(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	AmbientOcclusionIntensity = NewValue;
}

float FEMaterial::GetRoughnessMapIntensity() const
{
	return RoughnessMapIntensity;
}

void FEMaterial::SetRoughnessMapIntensity(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	RoughnessMapIntensity = NewValue;
}

float FEMaterial::GetMetalnessMapIntensity() const
{
	return MetalnessMapIntensity;
}

void FEMaterial::SetMetalnessMapIntensity(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	MetalnessMapIntensity = NewValue;
}

float FEMaterial::GetAmbientOcclusionMapIntensity() const
{
	return AmbientOcclusionMapIntensity;
}

void FEMaterial::SetAmbientOcclusionMapIntensity(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.001f;

	AmbientOcclusionMapIntensity = NewValue;
}

int FEMaterial::PlaceTextureInList(FETexture* Texture)
{
	if (Texture == nullptr)
		return -1;

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (Textures[i] == Texture)
			return static_cast<int>(i);
	}

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (Textures[i] == nullptr)
		{
			Textures[i] = Texture;
			return static_cast<int>(i);
		}
	}

	return -1;
}

FETexture* FEMaterial::GetAlbedoMap(const int SubMaterial)
{
	return GetSpecifiedMap(AlbedoBindingIndex, SubMaterial);
}

void FEMaterial::SetAlbedoMap(FETexture* Texture, const int SubMaterial)
{
	const int TextureIndex = PlaceTextureInList(Texture);
	if (TextureIndex == -1 && Texture == nullptr)
	{
		ClearTextureBinding(AlbedoBindingIndex, SubMaterial);
	}
	else
	{
		SetTextureBinding(AlbedoBindingIndex, TextureIndex, SubMaterial);
	}
}

void FEMaterial::SetAlbedoMap(const int TextureIndex, const int SubMaterial)
{
	if (Textures[TextureIndex] == nullptr)
		return;

	SetTextureBinding(AlbedoBindingIndex, TextureIndex, SubMaterial);
}

FETexture* FEMaterial::GetNormalMap(const int SubMaterial)
{
	return GetSpecifiedMap(NormalBindingIndex, SubMaterial);
}

void FEMaterial::SetNormalMap(FETexture* Texture, const int SubMaterial)
{
	const int TextureIndex = PlaceTextureInList(Texture);
	if (TextureIndex == -1 && Texture == nullptr)
	{
		ClearTextureBinding(NormalBindingIndex, SubMaterial);
	}
	else
	{
		SetTextureBinding(NormalBindingIndex, TextureIndex, SubMaterial);
	}
}

void FEMaterial::SetNormalMap(const int TextureIndex, const int SubMaterial)
{
	if (Textures[TextureIndex] == nullptr)
		return;

	SetTextureBinding(NormalBindingIndex, TextureIndex, SubMaterial);
}

FETexture* FEMaterial::GetAOMap(const int SubMaterial)
{
	return GetSpecifiedMap(AOBindingIndex, SubMaterial);
}

int FEMaterial::GetAOMapChannel(const int SubMaterial) const
{
	return TextureChannels[AOBindingIndex + SubMaterial * 6];
}

void FEMaterial::SetAOMap(FETexture* Texture, const int Channel, const int SubMaterial)
{
	const int TextureIndex = PlaceTextureInList(Texture);
	if (TextureIndex == -1 && Texture == nullptr)
	{
		ClearTextureBinding(AOBindingIndex, SubMaterial, Channel);
	}
	else
	{
		SetTextureBinding(AOBindingIndex, TextureIndex, SubMaterial, Channel);
	}
}

void FEMaterial::SetAOMap(const int TextureIndex, const int Channel, const int SubMaterial)
{
	if (Textures[TextureIndex] == nullptr)
		return;

	SetTextureBinding(AOBindingIndex, TextureIndex, SubMaterial, Channel);
}

FETexture* FEMaterial::GetRoughnessMap(const int SubMaterial)
{
	return GetSpecifiedMap(RoughnessBindingIndex, SubMaterial);
}

int FEMaterial::GetRoughnessMapChannel(const int SubMaterial) const
{
	return TextureChannels[RoughnessBindingIndex + SubMaterial * 6];
}

void FEMaterial::SetRoughnessMap(FETexture* Texture, const int Channel, const int SubMaterial)
{
	const int TextureIndex = PlaceTextureInList(Texture);
	if (TextureIndex == -1 && Texture == nullptr)
	{
		ClearTextureBinding(RoughnessBindingIndex, SubMaterial, Channel);
	}
	else
	{
		SetTextureBinding(RoughnessBindingIndex, TextureIndex, SubMaterial, Channel);
	}
}

void FEMaterial::SetRoughnessMap(const int TextureIndex, const int Channel, const int SubMaterial)
{
	if (Textures[TextureIndex] == nullptr)
		return;

	SetTextureBinding(RoughnessBindingIndex, TextureIndex, SubMaterial, Channel);
}

FETexture* FEMaterial::GetMetalnessMap(const int SubMaterial)
{
	return GetSpecifiedMap(MetalnessBindingIndex, SubMaterial);
}

int FEMaterial::GetMetalnessMapChannel(const int SubMaterial) const
{
	return TextureChannels[MetalnessBindingIndex + SubMaterial * 6];
}

void FEMaterial::SetMetalnessMap(FETexture* Texture, const int Channel, const int SubMaterial)
{
	const int TextureIndex = PlaceTextureInList(Texture);
	if (TextureIndex == -1 && Texture == nullptr)
	{
		ClearTextureBinding(MetalnessBindingIndex, SubMaterial, Channel);
	}
	else
	{
		SetTextureBinding(MetalnessBindingIndex, TextureIndex, SubMaterial, Channel);
	}
}

void FEMaterial::SetMetalnessMap(const int TextureIndex, const int Channel, const int SubMaterial)
{
	if (Textures[TextureIndex] == nullptr)
		return;

	SetTextureBinding(MetalnessBindingIndex, TextureIndex, SubMaterial, Channel);
}

FETexture* FEMaterial::GetDisplacementMap(const int SubMaterial)
{
	return GetSpecifiedMap(DisplacementBindingIndex, SubMaterial);
}

int FEMaterial::GetDisplacementMapChannel(const int SubMaterial) const
{
	return TextureChannels[DisplacementBindingIndex + SubMaterial * 6];
}

void FEMaterial::SetDisplacementMap(FETexture* Texture, const int Channel, const int SubMaterial)
{
	const int TextureIndex = PlaceTextureInList(Texture);
	if (TextureIndex == -1 && Texture == nullptr)
	{
		ClearTextureBinding(DisplacementBindingIndex, SubMaterial, Channel);
	}
	else
	{
		SetTextureBinding(DisplacementBindingIndex, TextureIndex, SubMaterial, Channel);
	}
}

void FEMaterial::SetDisplacementMap(const int TextureIndex, const int Channel, const int SubMaterial)
{
	if (Textures[TextureIndex] == nullptr)
		return;

	SetTextureBinding(DisplacementBindingIndex, TextureIndex, SubMaterial, Channel);
}

FETexture* FEMaterial::GetSpecifiedMap(const int BindingIndex, const int SubMaterial)
{
	if (SubMaterial > 1)
		return nullptr;

	if (BindingIndex > 5)
		return nullptr;

	if (TextureBindings[BindingIndex + SubMaterial * 6] == -1)
		return nullptr;

	// clean up messed up textureBindings.
	if (Textures[TextureBindings[BindingIndex + SubMaterial * 6]] == nullptr)
	{
		TextureBindings[BindingIndex + SubMaterial * 6] = -1;
		return nullptr;
	}

	return Textures[TextureBindings[BindingIndex + SubMaterial * 6]];
}

bool FEMaterial::AddTexture(FETexture* Texture)
{
	if (PlaceTextureInList(Texture) == -1)
		return false;

	return true;
}

void FEMaterial::RemoveTexture(FETexture* Texture)
{
	if (Texture == nullptr)
		return;

	int TextureIndex = -1;
	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (Textures[i] == Texture)
		{
			TextureIndex = static_cast<int>(i);
			break;
		}
	}

	if (TextureIndex == -1)
		return;

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (TextureBindings[i] == TextureIndex)
		{
			TextureBindings[i] = -1;
			TextureChannels[i] = -1;
		}
	}
	
	Textures[TextureIndex] = nullptr;
}

void FEMaterial::RemoveTexture(const int TextureIndex)
{
	if (TextureIndex == -1 || TextureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (TextureBindings[i] == TextureIndex)
		{
			TextureBindings[i] = -1;
			TextureChannels[i] = -1;
		}
	}

	Textures[TextureIndex] = nullptr;
}

void FEMaterial::ClearAllTexturesInfo()
{
	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		Textures[i] = nullptr;
		TextureBindings[i] = -1;
		TextureChannels[i] = -1;
	}
}

bool FEMaterial::IsTextureInList(const FETexture* Texture) const
{
	bool result = false;
	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (Textures[i] == nullptr)
			continue;

		if (Textures[i]->GetObjectID() == Texture->GetObjectID())
		{
			result = true;
			return result;
		}
	}

	return result;
}

bool FEMaterial::IsCompackPacking()
{
	if (!IsCompackPackingPossible())
		bCompackPacking = false;

	return bCompackPacking;
}

void FEMaterial::SetCompackPacking(const bool NewValue)
{
	if (NewValue && !IsCompackPackingPossible())
		return;

	bCompackPacking = NewValue;
}

bool FEMaterial::IsCompackPackingPossible()
{
	// All material properties should be available from one texture.
	if (GetAOMap() != nullptr && GetAOMap() == GetRoughnessMap() && GetAOMap() == GetMetalnessMap() && GetAOMap() == GetDisplacementMap())
		return true;

	return false;
}

void FEMaterial::SetTextureBinding(const int Index, const int TextureIndex, const int SubMaterial, const int Channel)
{
	if (SubMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
	{
		LOG.Add("FEMaterial::setTextureBinding with out of bound \"subMaterial\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (TextureIndex < 0 || TextureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
	{
		LOG.Add("FEMaterial::setTextureBinding with out of bound \"textureIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	const int FinalIndex = Index + SubMaterial * 6;
	if (FinalIndex < 0 || FinalIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
	{
		LOG.Add("FEMaterial::setTextureBinding with out of bound \"finalIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (Channel != -2)
	{
		TextureChannels[FinalIndex] = Channel;
	}

	SetDirtyFlag(true);
	TextureBindings[FinalIndex] = TextureIndex;
}

void FEMaterial::ClearTextureBinding(const int Index, const int SubMaterial, const int Channel)
{
	if (SubMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
	{
		LOG.Add("FEMaterial::setTextureBinding with out of bound \"subMaterial\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	const int FinalIndex = Index + SubMaterial * 6;
	if (FinalIndex < 0 || FinalIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
	{
		LOG.Add("FEMaterial::clearTextureBinding with out of bound \"index\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (Channel != -2)
	{
		TextureChannels[FinalIndex] = Channel;
	}

	SetDirtyFlag(true);
	TextureBindings[FinalIndex] = -1;
}

int FEMaterial::GetUsedTexturesCount() const
{
	int result = 0;
	for (size_t i = 0; i < Textures.size(); i++)
	{
		if (Textures[i] != nullptr)
			result++;
	}

	return result;
}

float FEMaterial::GetDisplacementMapIntensity() const
{
	return DisplacementMapIntensity;
}

void FEMaterial::SetDisplacementMapIntensity(const float NewValue)
{
	DisplacementMapIntensity = NewValue;
}

float FEMaterial::GetTiling() const
{
	return Tiling;
}

void FEMaterial::SetTiling(const float NewValue)
{
	Tiling = NewValue;
}