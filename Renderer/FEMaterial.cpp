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
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]->getTextureID());
	}
}

void FEMaterial::unBind()
{
	if (shaders.size() > 0)
		shaders[0]->stop();
}