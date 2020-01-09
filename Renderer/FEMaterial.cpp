#include "FEMaterial.h"
using namespace FocalEngine;

FEMaterial::FEMaterial(int ID, std::string Name)
{
	id = ID;
	name = Name;
}

FEMaterial::~FEMaterial()
{
	delete shader;

	for (size_t i = 0; i < textures.size(); i++)
	{
		delete textures[i];
	}
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

int FEMaterial::getID()
{
	return id;
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