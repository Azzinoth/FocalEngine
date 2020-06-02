#include "FEMaterial.h"
using namespace FocalEngine;

FEMaterial::FEMaterial(/*int ID*//*, std::string Name*/)
{
	//id = ID;
	//name = Name;
}

FEMaterial::~FEMaterial()
{
	delete shader;
}

void FEMaterial::bind()
{
	if (shader != nullptr)
		shader->start();

	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			textures[i]->bind(i);
	}
}

void FEMaterial::unBind()
{
	if (shader != nullptr)
		shader->stop();

	for (size_t i = 0; i < textures.size(); i++)
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

void FEMaterial::addTexture(FETexture* newTexture)
{
	if (!newTexture)
		return;

	textures.push_back(newTexture);
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

std::vector<std::string> FEMaterial::getTextureList()
{
	std::vector<std::string> result;
	for (size_t i = 0; i < textures.size(); i++)
	{
		result.push_back(textures[i]->getName());
	}

	return result;
}

FETexture* FEMaterial::getTexture(std::string name)
{
	for (size_t i = 0; i < textures.size(); i++)
	{
		if (name == textures[i]->getName())
			return textures[i];
	}

	return nullptr;
}

void FEMaterial::setTexture(FETexture* newTexture, std::string name)
{
	for (size_t i = 0; i < textures.size(); i++)
	{
		if (name == textures[i]->getName())
		{
			delete textures[i];
			textures[i] = newTexture;
			return;
		}
	}

	addTexture(newTexture);
}