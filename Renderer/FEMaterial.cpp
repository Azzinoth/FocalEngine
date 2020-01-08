#include "FEMaterial.h"
using namespace FocalEngine;

FEMaterial::FEMaterial(/*short ID, std::string Name*/)
{
	/*id = ID;
	name = Name;*/
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
	shader->start();

	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			textures[i]->bind(i);
	}
}

void FEMaterial::unBind()
{
	shader->stop();

	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			textures[i]->unBind();
	}
}

void FEMaterial::setParam(std::string name, int newData)
{
	shader->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, float newData)
{
	shader->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec2 newData)
{
	shader->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec3 newData)
{
	shader->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec4 newData)
{
	shader->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::mat4 newData)
{
	shader->getParam(name).updateData(newData);
}

void FEMaterial::addTexture(FETexture* newTexture)
{
	if (!newTexture)
		return;

	textures.push_back(newTexture);
}

short FEMaterial::getID()
{
	return id;
}

bool FEMaterial::addParameter(FEShaderParam newParameter)
{
	if (shader->userParams.find(newParameter.paramName) != shader->userParams.end())
		return false;

	shader->userParams[newParameter.paramName] = newParameter;
}

std::vector<std::string> FEMaterial::getParameterList()
{
	std::vector<std::string> result;

	auto iterator = shader->userParams.begin();
	while (iterator != shader->userParams.end())
	{
		result.push_back(iterator->second.getParamName());
		iterator++;
	}

	return result;
}

FEShaderParam* FEMaterial::getParametr(std::string name)
{
	if (shader->userParams.find(name) == shader->userParams.end())
		return nullptr;

	return &shader->userParams[name];
}

std::string FEMaterial::getName()
{
	return name;
}

void FEMaterial::setName(std::string newName)
{
	name = newName;
}