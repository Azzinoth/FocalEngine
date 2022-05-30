#include "FETexture.h"
using namespace FocalEngine;

FETexture::FETexture(std::string Name) : FEObject(FE_TEXTURE, Name)
{
	name = Name;
	getNewGLTextureID();
}

FETexture::FETexture(int Width, int Height, std::string Name) : FEObject(FE_TEXTURE, Name)
{
	name = Name;
	width = Width;
	height = Height;
	getNewGLTextureID();
	bind(0);
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	unBind();
}

FETexture::FETexture(GLint InternalFormat, GLenum Format, int Width, int Height, std::string Name) : FEObject(FE_TEXTURE, Name)
{
	width = Width;
	height = Height;
	internalFormat = InternalFormat;
	format = Format;
	getNewGLTextureID();
	bind(0);
	FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// to-do: it is needed for screen space effects but could interfere with other purposes
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	if (internalFormat == GL_RGBA16F || internalFormat == GL_RGB16F || internalFormat == GL_RGB32F || internalFormat == GL_RGBA32F)
		hdr = true;
}

void FETexture::getNewGLTextureID()
{
	FE_GL_ERROR(glGenTextures(1, &textureID));
	//LOG.add("Texture creation with textureID: " + std::to_string(textureID));
}

FETexture::~FETexture()
{
	for (size_t i = 0; i < noDeletingList.size(); i++)
	{
		if (noDeletingList[i] == textureID)
			return;
	}
	//LOG.add("Texture deletion with textureID: " + std::to_string(textureID));
	FE_GL_ERROR(glDeleteTextures(1, &textureID));
}

GLuint FETexture::getTextureID()
{
	return textureID;
}

void FETexture::bind(const unsigned int textureUnit)
{
	defaultTextureUnit = textureUnit;
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + textureUnit));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textureID));
}

void FETexture::unBind()
{
	if (defaultTextureUnit != -1)
	{
		FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + defaultTextureUnit));
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	}
}

std::string FETexture::getFileName()
{
	return fileName;
}

GLint FETexture::getInternalFormat()
{
	return internalFormat;
}

int FETexture::getWidth()
{
	return width;
}

int FETexture::getHeight()
{
	return height;
}

void FETexture::GPUAllocateTeture(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
{
	FE_GL_ERROR(glTexImage2D(target, level, internalformat, width, height, border, format, type, data));
#ifdef FE_GPUMEM_ALLOCATION_LOGING
	FELOG::getInstance().logError("Texture creation with width: " + std::to_string(width) + " height: " + std::to_string(height));
#endif
}

void FETexture::addToOnDeleteCallBackList(std::string objectID)
{
	callListOnDeleteFEObject.push_back(objectID);
}

void FETexture::eraseFromOnDeleteCallBackList(std::string objectID)
{
	for (size_t i = 0; i < callListOnDeleteFEObject.size(); i++)
	{
		if (callListOnDeleteFEObject[i] == objectID)
		{
			callListOnDeleteFEObject.erase(callListOnDeleteFEObject.begin() + i, callListOnDeleteFEObject.begin() + i + 1);
			break;
		}
	}
}

std::string FETexture::textureInternalFormatToString(GLint internalFormat)
{
	std::string result = "";

	if (internalFormat == GL_RGBA)
	{
		result += "GL_RGBA";
	}
	else if (internalFormat == GL_RED)
	{
		result += "GL_RED";
	}
	else if (internalFormat == GL_R16)
	{
		result += "GL_R16";
	}
	else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		result += "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";
	}
	else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		result += "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
	}
	
	return result;
}

std::vector<GLuint> FETexture::noDeletingList = std::vector<GLuint>();
void FETexture::addToNoDeletingList(GLuint textureID)
{
	noDeletingList.push_back(textureID);
}

unsigned char* FETexture::getRawData(size_t* rawDataSize)
{
	unsigned char* result = nullptr;
	if (rawDataSize != nullptr)
		*rawDataSize = 0;

	if (internalFormat != GL_RGBA &&
		internalFormat != GL_RED &&
		internalFormat != GL_R16 &&
		internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FETexture::getRawData internalFormat is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return result;
	}

	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textureID));

	if (internalFormat == GL_R16)
	{
		if (rawDataSize != nullptr)
			*rawDataSize = getWidth() * getHeight() * 2;
		result = new unsigned char[getWidth() * getHeight() * 2];
		glPixelStorei(GL_PACK_ALIGNMENT, 2);
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else if (internalFormat == GL_RED)
	{
		if (rawDataSize != nullptr)
			*rawDataSize = getWidth() * getHeight();
		result = new unsigned char[getWidth() * getHeight()];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else
	{
		if (rawDataSize != nullptr)
			*rawDataSize = getWidth() * getHeight() * 4;
		result = new unsigned char[getWidth() * getHeight() * 4];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, result));
	}

	return result;
}

void FETexture::updateRawData(unsigned char* newRawData, size_t mipCount)
{
	if (internalFormat != GL_RGBA &&
		internalFormat != GL_RED &&
		internalFormat != GL_R16 &&
		internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		internalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.add("FETexture::updateRawData internalFormat of texture is not supported", FE_LOG_ERROR, FE_LOG_SAVING);
		return;
	}

	if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT || internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		for (size_t i = 3; i < size_t(getWidth() * getHeight() * 4); i += 4)
		{
			if (newRawData[i] != 255)
			{
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			}
		}
	}

	FE_GL_ERROR(glDeleteTextures(1, &textureID));
	FE_GL_ERROR(glGenTextures(1, &textureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textureID));

	if (internalFormat == GL_RGBA)
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), GL_RGBA8, getWidth(), getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(newRawData)));
	}
	else if (internalFormat == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), GL_R8, getWidth(), getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getWidth(), getHeight(), GL_RED, GL_UNSIGNED_BYTE, (void*)(newRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else if (internalFormat == GL_R16)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), GL_R16, getWidth(), getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getWidth(), getHeight(), GL_RED, GL_UNSIGNED_SHORT, (void*)(newRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, int(mipCount), internalFormat, getWidth(), getHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(newRawData)));
	}
}