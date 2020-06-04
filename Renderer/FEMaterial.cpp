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
	delete shader;
}

void FEMaterial::bind()
{
	if (shader != nullptr)
		shader->start();

	if (albedoMap != nullptr) albedoMap->bind(0);
	if (normalMap != nullptr) normalMap->bind(1);
	
	/*for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			textures[i]->bind(i);
	}*/
}

void FEMaterial::unBind()
{
	if (shader != nullptr)
		shader->stop();

	if (albedoMap != nullptr) albedoMap->unBind();
	if (normalMap != nullptr) normalMap->unBind();
	/*for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			textures[i]->unBind();
	}*/
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

//void FEMaterial::addTexture(FETexture* newTexture)
//{
//	if (!newTexture)
//		return;
//
//	textures.push_back(newTexture);
//}

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

//std::vector<std::string> FEMaterial::getTextureList()
//{
//	std::vector<std::string> result;
//	for (size_t i = 0; i < textures.size(); i++)
//	{
//		result.push_back(textures[i]->getName());
//	}
//
//	return result;
//}
//
//FETexture* FEMaterial::getTexture(std::string name)
//{
//	for (size_t i = 0; i < textures.size(); i++)
//	{
//		if (name == textures[i]->getName())
//			return textures[i];
//	}
//
//	return nullptr;
//}
//
//void FEMaterial::setTexture(FETexture* newTexture, std::string name)
//{
//	for (size_t i = 0; i < textures.size(); i++)
//	{
//		if (name == textures[i]->getName())
//		{
//			delete textures[i];
//			textures[i] = newTexture;
//			return;
//		}
//	}
//
//	addTexture(newTexture);
//}