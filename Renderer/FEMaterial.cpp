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
			textures[i]->bind(i);
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
			return i;
	}

	for (size_t i = 0; i < FE_MAX_TEXTURES_PER_MATERIAL; i++)
	{
		if (textures[i] == nullptr)
		{
			textures[i] = texture;
			return i;
		}
	}

	return -1;
}

FETexture* FEMaterial::getAlbedoMap(int subMaterial)
{
	if (textureBindings[albedoBindingIndex + subMaterial * 6] == -1)
		return nullptr;
	return textures[textureBindings[albedoBindingIndex + subMaterial * 6]];
}

void FEMaterial::setAlbedoMap(FETexture* texture, int subMaterial)
{
	if (subMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
		return;
	int textureIndex = placeTextureInList(texture);
	if (textureIndex != -1 || (textureIndex == -1 && texture == nullptr))
		textureBindings[albedoBindingIndex + subMaterial * 6] = textureIndex;
}

void FEMaterial::setAlbedoMap(int textureIndex, int subMaterial)
{
	if (subMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
		return;
	if (textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;
	if (textures[textureIndex] == nullptr)
		return;

	textureBindings[albedoBindingIndex + subMaterial * 6] = textureIndex;
}

FETexture* FEMaterial::getNormalMap(int subMaterial)
{
	if (textureBindings[normalBindingIndex + subMaterial * 6] == -1)
		return nullptr;
	return textures[textureBindings[normalBindingIndex + subMaterial * 6]];
}

void FEMaterial::setNormalMap(FETexture* texture, int subMaterial)
{
	if (subMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
		return;
	int textureIndex = placeTextureInList(texture);
	if (textureIndex != -1 || (textureIndex == -1 && texture == nullptr))
		textureBindings[normalBindingIndex + subMaterial * 6] = textureIndex;
}

void FEMaterial::setNormalMap(int textureIndex, int subMaterial)
{
	if (subMaterial >= FE_MAX_SUBMATERIALS_PER_MATERIAL)
		return;
	if (textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;
	if (textures[textureIndex] == nullptr)
		return;

	textureBindings[normalBindingIndex + subMaterial * 6] = textureIndex;
}

FETexture* FEMaterial::getAOMap(int subMaterial)
{
	if (textureBindings[AOBindingIndex + subMaterial * 6] == -1)
		return nullptr;
	return textures[textureBindings[AOBindingIndex + subMaterial * 6]];
}

int FEMaterial::getAOMapChannel(int subMaterial)
{
	return textureChannels[AOBindingIndex + subMaterial * 6];
}

void FEMaterial::setAOMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex != -1 || (textureIndex == -1 && texture == nullptr))
	{
		textureBindings[AOBindingIndex + subMaterial * 6] = textureIndex;
		textureChannels[AOBindingIndex + subMaterial * 6] = channel;
	}
}

void FEMaterial::setAOMap(int textureIndex, int channel, int subMaterial)
{
	if (textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;
	if (textures[textureIndex] == nullptr)
		return;

	textureBindings[AOBindingIndex + subMaterial * 6] = textureIndex;
	textureChannels[AOBindingIndex + subMaterial * 6] = channel;
}

FETexture* FEMaterial::getRoughtnessMap(int subMaterial)
{
	if (textureBindings[roughtnessBindingIndex + subMaterial * 6] == -1)
		return nullptr;
	return textures[textureBindings[roughtnessBindingIndex + subMaterial * 6]];
}

int FEMaterial::getRoughtnessMapChannel(int subMaterial)
{
	return textureChannels[roughtnessBindingIndex + subMaterial * 6];
}

void FEMaterial::setRoughtnessMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex != -1 || (textureIndex == -1 && texture == nullptr))
	{
		textureBindings[roughtnessBindingIndex + subMaterial * 6] = textureIndex;
		textureChannels[roughtnessBindingIndex + subMaterial * 6] = channel;
	}
}

void FEMaterial::setRoughtnessMap(int textureIndex, int channel, int subMaterial)
{
	if (textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;
	if (textures[textureIndex] == nullptr)
		return;

	textureBindings[roughtnessBindingIndex + subMaterial * 6] = textureIndex;
	textureChannels[roughtnessBindingIndex + subMaterial * 6] = channel;
}

FETexture* FEMaterial::getMetalnessMap(int subMaterial)
{
	if (textureBindings[metalnessBindingIndex + subMaterial * 6] == -1)
		return nullptr;
	return textures[textureBindings[metalnessBindingIndex + subMaterial * 6]];
}

int FEMaterial::getMetalnessMapChannel(int subMaterial)
{
	return textureChannels[metalnessBindingIndex + subMaterial * 6];
}

void FEMaterial::setMetalnessMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex != -1 || (textureIndex == -1 && texture == nullptr))
	{
		textureBindings[metalnessBindingIndex + subMaterial * 6] = textureIndex;
		textureChannels[metalnessBindingIndex + subMaterial * 6] = channel;
	}
}

void FEMaterial::setMetalnessMap(int textureIndex, int channel, int subMaterial)
{
	if (textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;
	if (textures[textureIndex] == nullptr)
		return;

	textureBindings[metalnessBindingIndex + subMaterial * 6] = textureIndex;
	textureChannels[metalnessBindingIndex + subMaterial * 6] = channel;
}

FETexture* FEMaterial::getDisplacementMap(int subMaterial)
{
	if (textureBindings[displacementBindingIndex + subMaterial * 6] == -1)
		return nullptr;
	return textures[textureBindings[displacementBindingIndex + subMaterial * 6]];
}

int FEMaterial::getDisplacementMapChannel(int subMaterial)
{
	return textureChannels[displacementBindingIndex + subMaterial * 6];
}

void FEMaterial::setDisplacementMap(FETexture* texture, int channel, int subMaterial)
{
	int textureIndex = placeTextureInList(texture);
	if (textureIndex != -1 || (textureIndex == -1 && texture == nullptr))
	{
		textureBindings[displacementBindingIndex + subMaterial * 6] = textureIndex;
		textureChannels[displacementBindingIndex + subMaterial * 6] = channel;
	}
}

void FEMaterial::setDisplacementMap(int textureIndex, int channel, int subMaterial)
{
	if (textureIndex >= FE_MAX_TEXTURES_PER_MATERIAL)
		return;
	if (textures[textureIndex] == nullptr)
		return;

	textureBindings[displacementBindingIndex + subMaterial * 6] = textureIndex;
	textureChannels[displacementBindingIndex + subMaterial * 6] = channel;
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
			textureIndex = i;
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