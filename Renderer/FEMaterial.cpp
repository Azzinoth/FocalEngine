#include "FEMaterial.h"
#include <algorithm>
using namespace FocalEngine;

FEMaterial::FEMaterial(std::string Name) : FEObject(FE_MATERIAL, Name)
{
	name = Name;

	textures.resize(FE_MAX_TEXTURES_PER_MATERIAL);
	std::for_each(textures.begin(), textures.end(), [](FETexture*& item) {
		item = nullptr;
	});

	textureBindings.resize(FE_MAX_TEXTURES_PER_MATERIAL);
	std::for_each(textureBindings.begin(), textureBindings.end(), [](int& item) {
		item = -1;
	});

	textureChannels.resize(FE_MAX_TEXTURES_PER_MATERIAL);
	std::for_each(textureChannels.begin(), textureChannels.end(), [](int& item) {
		item = -1;
	});
}

FEMaterial::~FEMaterial()
{
}

void FEMaterial::bind()
{
	if (shader != nullptr)
		shader->start();

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textures[i] != nullptr)
			textures[i]->bind(int(i));
	}

	// #fix such specific if statement in this class is not clean coding
	if (shader->getParameter("baseColor") != nullptr)
		shader->getParameter("baseColor")->updateData(baseColor);
}

void FEMaterial::unBind()
{
	if (shader != nullptr)
		shader->stop();

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textures[i] != nullptr)
			textures[i]->unBind();
	}
}

void FEMaterial::setParam(std::string name, int newData)
{
	shader->getParameter(name)->updateData(newData);
}

void FEMaterial::setParam(std::string name, float newData)
{
	shader->getParameter(name)->updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec2 newData)
{
	shader->getParameter(name)->updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec3 newData)
{
	shader->getParameter(name)->updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec4 newData)
{
	shader->getParameter(name)->updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::mat4 newData)
{
	shader->getParameter(name)->updateData(newData);
}

void FEMaterial::addParameter(FEShaderParam newParameter)
{
	shader->addParameter(newParameter);
}

std::vector<std::string> FEMaterial::getParameterList()
{
	return shader->getParameterList();
}

FEShaderParam* FEMaterial::getParameter(std::string name)
{
	return shader->getParameter(name);
}

glm::vec3 FEMaterial::getBaseColor()
{
	return baseColor;
}

// Only infuence color of object if shader with such uniform is applied.
void FEMaterial::setBaseColor(glm::vec3 newBaseColor)
{
	baseColor = newBaseColor;
}

float FEMaterial::getMetalness()
{
	return metalness;
}

void FEMaterial::setMetalness(float newMetalness)
{
	if (newMetalness > 1.0f)
		newMetalness = 1.0f;

	if (newMetalness < 0.0f)
		newMetalness = 0.0f;

	metalness = newMetalness;
}

float FEMaterial::getRoughtness()
{
	return roughtness;
}

void FEMaterial::setRoughtness(float newRoughtness)
{
	if (newRoughtness > 1.0f)
		newRoughtness = 1.0f;

	if (newRoughtness < 0.0f)
		newRoughtness = 0.0f;

	roughtness = newRoughtness;
}

float FEMaterial::getNormalMapIntensity()
{
	return normalMapIntensity;
}

void FEMaterial::setNormalMapIntensity(float newNormalMapIntensity)
{
	if (newNormalMapIntensity > 1.0f)
		newNormalMapIntensity = 1.0f;

	if (newNormalMapIntensity < 0.0f)
		newNormalMapIntensity = 0.0f;

	normalMapIntensity = newNormalMapIntensity;
}

float FEMaterial::getAmbientOcclusionIntensity()
{
	return ambientOcclusionIntensity;
}

void FEMaterial::setAmbientOcclusionIntensity(float newAmbientOcclusionIntensity)
{
	if (newAmbientOcclusionIntensity < 0.0f)
		newAmbientOcclusionIntensity = 0.0f;

	ambientOcclusionIntensity = newAmbientOcclusionIntensity;
}

float FEMaterial::getRoughtnessMapIntensity()
{
	return roughtnessMapIntensity;
}

void FEMaterial::setRoughtnessMapIntensity(float newRoughtnessMapIntensity)
{
	if (newRoughtnessMapIntensity < 0.0f)
		newRoughtnessMapIntensity = 0.0f;

	roughtnessMapIntensity = newRoughtnessMapIntensity;
}

float FEMaterial::getMetalnessMapIntensity()
{
	return metalnessMapIntensity;
}

void FEMaterial::setMetalnessMapIntensity(float newMetalnessMapIntensity)
{
	if (newMetalnessMapIntensity < 0.0f)
		newMetalnessMapIntensity = 0.0f;

	metalnessMapIntensity = newMetalnessMapIntensity;
}

float FEMaterial::getAmbientOcclusionMapIntensity()
{
	return ambientOcclusionMapIntensity;
}

void FEMaterial::setAmbientOcclusionMapIntensity(float newAmbientOcclusionMapIntensity)
{
	if (newAmbientOcclusionMapIntensity <= 0.0f)
		newAmbientOcclusionMapIntensity = 0.001f;

	ambientOcclusionMapIntensity = newAmbientOcclusionMapIntensity;
}

int FEMaterial::placeTextureInList(FETexture* texture)
{
	if (texture == nullptr)
		return -1;

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textures[i] == texture)
			return int(i);
	}

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textures[i] == nullptr)
		{
			textures[i] = texture;
			return int(i);
		}
	}

	return -1;
}

FETexture* FEMaterial::getAlbedoMap(int subMaterial)
{
	return getSpecifiedMap(albedoBindingIndex, subMaterial);
}

void FEMaterial::setAlbedoMap(FETexture* texture, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex == -1 && texture == nullptr)
	{
		clearTextureBinding(albedoBindingIndex, subMaterial);
	}
	else
	{
		setTextureBinding(albedoBindingIndex, textureIndex, subMaterial);
	}
}

void FEMaterial::setAlbedoMap(int textureIndex, int subMaterial)
{
	if (textures[textureIndex] == nullptr)
		return;

	setTextureBinding(albedoBindingIndex, textureIndex, subMaterial);
}

FETexture* FEMaterial::getNormalMap(int subMaterial)
{
	return getSpecifiedMap(normalBindingIndex, subMaterial);
}

void FEMaterial::setNormalMap(FETexture* texture, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex == -1 && texture == nullptr)
	{
		clearTextureBinding(normalBindingIndex, subMaterial);
	}
	else
	{
		setTextureBinding(normalBindingIndex, textureIndex, subMaterial);
	}
}

void FEMaterial::setNormalMap(int textureIndex, int subMaterial)
{
	if (textures[textureIndex] == nullptr)
		return;

	setTextureBinding(normalBindingIndex, textureIndex, subMaterial);
}

FETexture* FEMaterial::getAOMap(int subMaterial)
{
	return getSpecifiedMap(AOBindingIndex, subMaterial);
}

int FEMaterial::getAOMapChannel(int subMaterial)
{
	return textureChannels[AOBindingIndex + subMaterial * 6];
}

void FEMaterial::setAOMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex == -1 && texture == nullptr)
	{
		clearTextureBinding(AOBindingIndex, subMaterial, channel);
	}
	else
	{
		setTextureBinding(AOBindingIndex, textureIndex, subMaterial, channel);
	}
}

void FEMaterial::setAOMap(int textureIndex, int channel, int subMaterial)
{
	if (textures[textureIndex] == nullptr)
		return;

	setTextureBinding(AOBindingIndex, textureIndex, subMaterial, channel);
}

FETexture* FEMaterial::getRoughtnessMap(int subMaterial)
{
	return getSpecifiedMap(roughtnessBindingIndex, subMaterial);
}

int FEMaterial::getRoughtnessMapChannel(int subMaterial)
{
	return textureChannels[roughtnessBindingIndex + subMaterial * 6];
}

void FEMaterial::setRoughtnessMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex == -1 && texture == nullptr)
	{
		clearTextureBinding(roughtnessBindingIndex, subMaterial, channel);
	}
	else
	{
		setTextureBinding(roughtnessBindingIndex, textureIndex, subMaterial, channel);
	}
}

void FEMaterial::setRoughtnessMap(int textureIndex, int channel, int subMaterial)
{
	if (textures[textureIndex] == nullptr)
		return;

	setTextureBinding(roughtnessBindingIndex, textureIndex, subMaterial, channel);
}

FETexture* FEMaterial::getMetalnessMap(int subMaterial)
{
	return getSpecifiedMap(metalnessBindingIndex, subMaterial);
}

int FEMaterial::getMetalnessMapChannel(int subMaterial)
{
	return textureChannels[metalnessBindingIndex + subMaterial * 6];
}

void FEMaterial::setMetalnessMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex == -1 && texture == nullptr)
	{
		clearTextureBinding(metalnessBindingIndex, subMaterial, channel);
	}
	else
	{
		setTextureBinding(metalnessBindingIndex, textureIndex, subMaterial, channel);
	}
}

void FEMaterial::setMetalnessMap(int textureIndex, int channel, int subMaterial)
{
	if (textures[textureIndex] == nullptr)
		return;

	setTextureBinding(metalnessBindingIndex, textureIndex, subMaterial, channel);
}

FETexture* FEMaterial::getDisplacementMap(int subMaterial)
{
	return getSpecifiedMap(displacementBindingIndex, subMaterial);
}

int FEMaterial::getDisplacementMapChannel(int subMaterial)
{
	return textureChannels[displacementBindingIndex + subMaterial * 6];
}

void FEMaterial::setDisplacementMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex == -1 && texture == nullptr)
	{
		clearTextureBinding(displacementBindingIndex, subMaterial, channel);
	}
	else
	{
		setTextureBinding(displacementBindingIndex, textureIndex, subMaterial, channel);
	}
}

void FEMaterial::setDisplacementMap(int textureIndex, int channel, int subMaterial)
{
	if (textures[textureIndex] == nullptr)
		return;

	setTextureBinding(displacementBindingIndex, textureIndex, subMaterial, channel);
}

FETexture* FEMaterial::getSpecifiedMap(int bindingIndex, int subMaterial)
{
	if (subMaterial > 1)
		return nullptr;

	if (bindingIndex > 5)
		return nullptr;

	if (textureBindings[bindingIndex + subMaterial * 6] == -1)
		return nullptr;

	// clean up messed up textureBindings.
	if (textures[textureBindings[bindingIndex + subMaterial * 6]] == nullptr)
	{
		textureBindings[bindingIndex + subMaterial * 6] = -1;
		return nullptr;
	}

	return textures[textureBindings[bindingIndex + subMaterial * 6]];
}

bool FEMaterial::addTexture(FETexture* texture)
{
	if (placeTextureInList(texture) == -1)
		return false;

	return true;
}

void FEMaterial::removeTexture(FETexture* texture)
{
	if (texture == nullptr)
		return;

	int textureIndex = -1;
	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textures[i] == texture)
		{
			textureIndex = int(i);
			break;
		}
	}

	if (textureIndex == -1)
		return;

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textureBindings[i] == textureIndex)
		{
			textureBindings[i] = -1;
			textureChannels[i] = -1;
		}
	}
	
	textures[textureIndex] = nullptr;
}

void FEMaterial::removeTexture(int textureIndex)
{
	if (textureIndex == -1 || textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textureBindings[i] == textureIndex)
		{
			textureBindings[i] = -1;
			textureChannels[i] = -1;
		}
	}

	textures[textureIndex] = nullptr;
}

void FEMaterial::clearAllTexturesInfo()
{
	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		textures[i] = nullptr;
		textureBindings[i] = -1;
		textureChannels[i] = -1;
	}
}

bool FEMaterial::isTextureInList(FETexture* texture)
{
	bool result = false;
	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textures[i] == nullptr)
			continue;

		if (textures[i]->getObjectID() == texture->getObjectID())
		{
			result = true;
			return result;
		}
	}

	return result;
}

bool FEMaterial::isCompackPacking()
{
	if (!isCompackPackingPossible())
		compackPacking = false;

	return compackPacking;
}

void FEMaterial::setCompackPacking(bool newValue)
{
	if (newValue && !isCompackPackingPossible())
		return;

	compackPacking = newValue;
}

bool FEMaterial::isCompackPackingPossible()
{
	// All material properties should be available from one texture.
	if (getAOMap() != nullptr && getAOMap() == getRoughtnessMap() && getAOMap() == getMetalnessMap() && getAOMap() == getDisplacementMap())
		return true;

	return false;
}

void FEMaterial::setTextureBinding(int index, int textureIndex, int subMaterial, int channel)
{
	if (subMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
	{
		LOG.add("FEMaterial::setTextureBinding with out of bound \"subMaterial\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (textureIndex < 0 || textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
	{
		LOG.add("FEMaterial::setTextureBinding with out of bound \"textureIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	int finalIndex = index + subMaterial * 6;
	if (finalIndex < 0 || finalIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
	{
		LOG.add("FEMaterial::setTextureBinding with out of bound \"finalIndex\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (channel != -2)
	{
		textureChannels[finalIndex] = channel;
	}

	setDirtyFlag(true);
	textureBindings[finalIndex] = textureIndex;
}

void FEMaterial::clearTextureBinding(int index, int subMaterial, int channel)
{
	if (subMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
	{
		LOG.add("FEMaterial::setTextureBinding with out of bound \"subMaterial\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	int finalIndex = index + subMaterial * 6;
	if (finalIndex < 0 || finalIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
	{
		LOG.add("FEMaterial::clearTextureBinding with out of bound \"index\"", FE_LOG_WARNING, FE_LOG_RENDERING);
		return;
	}

	if (channel != -2)
	{
		textureChannels[finalIndex] = channel;
	}

	setDirtyFlag(true);
	textureBindings[finalIndex] = -1;
}

int FEMaterial::getUsedTexturesCount()
{
	int result = 0;
	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			result++;
	}

	return result;
}

float FEMaterial::getDisplacementMapIntensity()
{
	return displacementMapIntensity;
}

void FEMaterial::setDisplacementMapIntensity(float newValue)
{
	displacementMapIntensity = newValue;
}

float FEMaterial::getTiling()
{
	return tiling;
}

void FEMaterial::setTiling(float newValue)
{
	tiling = newValue;
}