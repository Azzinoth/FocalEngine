#include "FETexture.h"

FocalEngine::FETexture::FETexture(char* fileName, std::string Name)
{
	textureID = png_texture_load(fileName);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // smooth textures not blocky :)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.8f);

	setName(Name);
}

FocalEngine::FETexture::FETexture(std::string Name)
{
	setName(Name);
}

FocalEngine::FETexture::~FETexture()
{
	glDeleteTextures(1, &textureID);
}

GLuint FocalEngine::FETexture::getTextureID()
{
	return textureID;
}

std::string FocalEngine::FETexture::getName()
{
	return name;
}

void FocalEngine::FETexture::setName(std::string newName)
{
	name = newName;
}