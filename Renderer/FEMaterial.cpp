#include "FEMaterial.h"
using namespace FocalEngine;

FEMaterial::FEMaterial()
{
}

FEMaterial::~FEMaterial()
{
	for (size_t i = 0; i < shaders.size(); i++)
	{
		delete shaders[i];
	}

	for (size_t i = 0; i < textures.size(); i++)
	{
		delete textures[i];
	}
}

void FEMaterial::bind()
{
	if (shaders.size() > 0)
		shaders[0]->start();

	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			textures[i]->bind(i);
		/*glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]->getTextureID());*/
	}
}

void FEMaterial::unBind()
{
	if (shaders.size() > 0)
		shaders[0]->stop();

	for (size_t i = 0; i < textures.size(); i++)
	{
		if (textures[i] != nullptr)
			textures[i]->unBind();
	}
}

void FEMaterial::setParam(std::string name, int newData)
{
	shaders[0]->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, float newData)
{
	shaders[0]->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec2 newData)
{
	shaders[0]->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec3 newData)
{
	shaders[0]->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::vec4 newData)
{
	shaders[0]->getParam(name).updateData(newData);
}

void FEMaterial::setParam(std::string name, glm::mat4 newData)
{
	shaders[0]->getParam(name).updateData(newData);
}

void FEMaterial::addTexture(FETexture* newTexture)
{
	if (!newTexture)
		return;

	textures.push_back(newTexture);
}