#include "FEMaterial.h"
using namespace FocalEngine;

FEMaterial::FEMaterial(std::string Name) : FEAsset(FE_MATERIAL, Name)
{
	name = Name;
	albedoMap = nullptr;
	normalMap = nullptr;
	roughtnessMap = nullptr;
	metalnessMap = nullptr;
	AOMap = nullptr;
	displacementMap = nullptr;
}

FEMaterial::~FEMaterial()
{
}

void FEMaterial::bind()
{
	if (shader != nullptr)
		shader->start();

	if (albedoMap != nullptr) albedoMap->bind(0);
	if (normalMap != nullptr) normalMap->bind(1);
	if (AOMap != nullptr) AOMap->bind(2);
	if (roughtnessMap != nullptr) roughtnessMap->bind(3);
	if (metalnessMap != nullptr) metalnessMap->bind(4);
	if (MRAOMap != nullptr) MRAOMap->bind(5);
	if (displacementMap != nullptr) displacementMap->bind(6);

	// #fix such specific if statement in this class is not clean coding
	if (shader->getParameter("baseColor") != nullptr)
		shader->getParameter("baseColor")->updateData(baseColor);
}

void FEMaterial::unBind()
{
	if (shader != nullptr)
		shader->stop();

	if (albedoMap != nullptr) albedoMap->unBind();
	if (normalMap != nullptr) normalMap->unBind();
	if (roughtnessMap != nullptr) roughtnessMap->unBind();
	if (metalnessMap != nullptr) metalnessMap->unBind();
	if (AOMap != nullptr) AOMap->unBind();
	if (displacementMap != nullptr) displacementMap->unBind();
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

std::string FEMaterial::getName()
{
	return name;
}

void FEMaterial::setName(std::string newName)
{
	name = newName;
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