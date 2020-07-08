#include "FEMaterial.h"
using namespace FocalEngine;

FEMaterial::FEMaterial()
{
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
	if (displacementMap != nullptr) displacementMap->bind(5);
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