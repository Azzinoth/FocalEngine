#include "FETexture.h"
#include "../ResourceManager/FEResourceManager.h"
#include "glm/gtc/packing.hpp"
using namespace FocalEngine;

FETexture::FETexture(const std::string Name, FE_TEXTURE_TYPE TextureType) : FEObject(FE_TEXTURE, Name)
{
	this->Type = TextureType;
	this->Name = Name;

	GetNewGlTextureID();
}

FETexture::FETexture(const int Width, const int Height, const std::string Name, FE_TEXTURE_TYPE TextureType) : FEObject(FE_TEXTURE, Name)
{
	this->Type = TextureType;
	this->Name = Name;
	this->Width = Width;
	this->Height = Height;

	GetNewGlTextureID();
	Bind(0);
	RESOURCE_MANAGER.Upload2DTextureDataToGPU(this, 0, GL_RGB, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	UnBind();
}

FETexture::FETexture(const GLint InternalFormat, const GLenum Format, const int Width, const int Height, const std::string Name, FE_TEXTURE_TYPE TextureType) : FEObject(FE_TEXTURE, Name)
{
	this->Type = TextureType;
	this->Width = Width;
	this->Height = Height;
	this->InternalFormat = InternalFormat;
	this->Format = Format;

	GetNewGlTextureID();
	Bind(0);
	RESOURCE_MANAGER.Upload2DTextureDataToGPU(this, 0, InternalFormat, Width, Height, Format, GL_UNSIGNED_BYTE, nullptr);
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// to-do: it is needed for screen space effects but could interfere with other purposes
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	if (InternalFormat == GL_RGBA16F || InternalFormat == GL_RGB16F || InternalFormat == GL_RGB32F || InternalFormat == GL_RGBA32F)
		bHDR = true;
}

FE_TEXTURE_TYPE FETexture::GetType() const
{
	return Type;
}

void FETexture::GetNewGlTextureID()
{
	FE_GL_ERROR(glGenTextures(1, &TextureID));
}

FETexture::~FETexture()
{
	for (size_t i = 0; i < PreventAutoDeletionList.size(); i++)
	{
		if (PreventAutoDeletionList[i] == TextureID)
			return;
	}
#ifdef FE_GPUMEM_ALLOCATION_LOGGING
	LOG.Add("Texture deleted with width: " + std::to_string(Width) + " height: " + std::to_string(Height), "FE_GPU_ALLOCATIONS");
#endif
	FE_GL_ERROR(glDeleteTextures(1, &TextureID));
}

GLuint FETexture::GetTextureID()
{
	return TextureID;
}

void FETexture::Bind(const unsigned int TextureUnit)
{
	DefaultTextureUnit = TextureUnit;
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + TextureUnit));

	if (Type == FE_TEXTURE_TYPE::FE_TEXTURE_2D)
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, TextureID));

	if (Type == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_3D, TextureID));
}

void FETexture::UnBind()
{
	if (DefaultTextureUnit != -1)
	{
		FE_GL_ERROR(glActiveTexture(GL_TEXTURE0 + DefaultTextureUnit));
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	}
}

std::string FETexture::GetFileName()
{
	return FileName;
}

GLint FETexture::GetInternalFormat()
{
	return InternalFormat;
}

int FETexture::GetWidth()
{
	return Width;
}

int FETexture::GetHeight()
{
	return Height;
}

int FETexture::GetDepth()
{
	return Depth;
}

glm::vec4 FETexture::GetMinValue()
{
	return MinValue;
}

glm::vec4 FETexture::GetMaxValue()
{
	return MaxValue;
}

void FETexture::AddToOnDeleteCallBackList(const std::string ObjectID)
{
	CallListOnDeleteFEObject.push_back(ObjectID);
}

void FETexture::EraseFromOnDeleteCallBackList(const std::string ObjectID)
{
	for (size_t i = 0; i < CallListOnDeleteFEObject.size(); i++)
	{
		if (CallListOnDeleteFEObject[i] == ObjectID)
		{
			CallListOnDeleteFEObject.erase(CallListOnDeleteFEObject.begin() + i, CallListOnDeleteFEObject.begin() + i + 1);
			break;
		}
	}
}

std::string FETexture::TextureInternalFormatToString(const GLint InternalFormat)
{
	std::string Result;

	if (InternalFormat == GL_RGBA)
	{
		Result += "GL_RGBA";
	}
	else if (InternalFormat == GL_RED)
	{
		Result += "GL_RED";
	}
	else if (InternalFormat == GL_R16)
	{
		Result += "GL_R16";
	}
	else if (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		Result += "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";
	}
	else if (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		Result += "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
	}
	
	return Result;
}

std::vector<GLuint> FETexture::PreventAutoDeletionList = std::vector<GLuint>();
void FETexture::MarkAsPersistent(const GLuint TextureID)
{
	PreventAutoDeletionList.push_back(TextureID);
}

unsigned char* FETexture::GetRawData(size_t* RawDataSize)
{
	unsigned char* Result = nullptr;
	if (RawDataSize != nullptr)
		*RawDataSize = 0;

	if (InternalFormat != GL_RGBA &&
		InternalFormat != GL_RGB &&
		InternalFormat != GL_RED &&
		InternalFormat != GL_R16 &&
		InternalFormat != GL_RG16F &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT &&
		InternalFormat != GL_RGBA16F &&
		InternalFormat != GL_DEPTH24_STENCIL8 &&
		InternalFormat != GL_DEPTH_COMPONENT32 &&
		InternalFormat != GL_R32F)
	{
		LOG.Add("FETexture::GetRawData InternalFormat is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return Result;
	}

	bool b3DTexture = Type == FE_TEXTURE_TYPE::FE_TEXTURE_3D;

	Bind();

	const size_t PixelsCount = Width * Height * (b3DTexture ? Depth : 1);
	GLenum AppropriateTarget = b3DTexture ? GL_TEXTURE_3D : GL_TEXTURE_2D;

	if (InternalFormat == GL_R32F)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * sizeof(float);
		Result = new unsigned char[PixelsCount * sizeof(float)];
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_RED, GL_FLOAT, Result));
	}
	else if (InternalFormat == GL_RG16F)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * 2 * sizeof(unsigned short);
		Result = new unsigned char[PixelsCount * 2 * sizeof(unsigned short)];
		glPixelStorei(GL_PACK_ALIGNMENT, 2);
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_RG, GL_HALF_FLOAT, Result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_DEPTH_COMPONENT32)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * sizeof(float);
		Result = new unsigned char[PixelsCount * sizeof(float)];
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_DEPTH_COMPONENT, GL_FLOAT, Result));
	}
	else if (InternalFormat == GL_DEPTH24_STENCIL8)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * 4;
		Result = new unsigned char[PixelsCount * 4];
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, Result));
	}
	else if (InternalFormat == GL_RGBA16F)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * 4 * sizeof(unsigned short);

		Result = new unsigned char[PixelsCount * 4 * sizeof(unsigned short)];
		glPixelStorei(GL_PACK_ALIGNMENT, 2);
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_RGBA, GL_HALF_FLOAT, Result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_R16)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * 2;
		Result = new unsigned char[PixelsCount * 2];
		glPixelStorei(GL_PACK_ALIGNMENT, 2);
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_RED, GL_UNSIGNED_SHORT, Result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_RED)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount;
		Result = new unsigned char[PixelsCount];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_RED, GL_UNSIGNED_BYTE, Result));
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	// Check GL_COMPRESSED_RGBA_S3TC_DXT5_EXT and GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	else if (InternalFormat == GL_RGBA || InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT || InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * 4;
		Result = new unsigned char[PixelsCount * 4];
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_RGBA, GL_UNSIGNED_BYTE, Result));
	}
	else if (InternalFormat == GL_RGB)
	{
		if (RawDataSize != nullptr)
			*RawDataSize = PixelsCount * 3;
		Result = new unsigned char[PixelsCount * 3];
		FE_GL_ERROR(glGetTexImage(AppropriateTarget, 0, GL_RGB, GL_UNSIGNED_BYTE, Result));
	}

	return Result;
}

void FETexture::UpdateRawData(unsigned char* NewRawData, const size_t MipmapCount)
{
	if (InternalFormat != GL_RGBA &&
		InternalFormat != GL_RED &&
		InternalFormat != GL_R16 &&
		InternalFormat != GL_R32F &&
		InternalFormat != GL_RGBA16F &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
		InternalFormat != GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		LOG.Add("FETexture::updateRawData internalFormat of texture is not supported", "FE_LOG_SAVING", FE_LOG_ERROR);
		return;
	}

	if (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT || InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		for (size_t i = 3; i < static_cast<size_t>(GetWidth() * GetHeight() * 4); i += 4)
		{
			if (NewRawData[i] != 255)
			{
				InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			}
		}
	}

	FE_GL_ERROR(glDeleteTextures(1, &TextureID));
	FE_GL_ERROR(glGenTextures(1, &TextureID));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, TextureID));

	if (InternalFormat == GL_RGBA16F)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipmapCount), GL_RGBA16F, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RGBA, GL_HALF_FLOAT, (void*)(NewRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_RGBA)
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipmapCount), GL_RGBA8, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(NewRawData)));
	}
	else if (InternalFormat == GL_RED)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipmapCount), GL_R8, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RED, GL_UNSIGNED_BYTE, (void*)(NewRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_R16)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipmapCount), GL_R16, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RED, GL_UNSIGNED_SHORT, (void*)(NewRawData)));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
	else if (InternalFormat == GL_R32F)
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipmapCount), GL_R32F, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RED, GL_FLOAT, (void*)(NewRawData)));
	}
	else
	{
		FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipmapCount), InternalFormat, GetWidth(), GetHeight()));
		FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetWidth(), GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (void*)(NewRawData)));
	}

	UpdateMinMaxValues(NewRawData);
}

template <typename ComponentType>
static void AccumulateMinMax(const ComponentType* Data, const size_t PixelCount, const int ChannelCount, glm::vec4& OutMin, glm::vec4& OutMax)
{
	for (size_t Pixel = 0; Pixel < PixelCount; Pixel++)
	{
		for (int Channel = 0; Channel < ChannelCount; Channel++)
		{
			const float Value = static_cast<float>(Data[Pixel * ChannelCount + Channel]);
			if (Value < OutMin[Channel])
				OutMin[Channel] = Value;
			if (Value > OutMax[Channel])
				OutMax[Channel] = Value;
		}
	}
}

static void AccumulateMinMaxHalf(const unsigned short* Data, const size_t PixelCount, const int ChannelCount, glm::vec4& OutMin, glm::vec4& OutMax)
{
	for (size_t Pixel = 0; Pixel < PixelCount; Pixel++)
	{
		for (int Channel = 0; Channel < ChannelCount; Channel++)
		{
			const float Value = glm::unpackHalf1x16(Data[Pixel * ChannelCount + Channel]);
			if (Value < OutMin[Channel])
				OutMin[Channel] = Value;
			if (Value > OutMax[Channel])
				OutMax[Channel] = Value;
		}
	}
}

void FETexture::UpdateMinMaxValues(const unsigned char* RawData)
{
	// Re-seed the empty interval on every call so re-computation stays correct.
	MinValue = glm::vec4(std::numeric_limits<float>::max());
	MaxValue = glm::vec4(-std::numeric_limits<float>::max());

	if (RawData == nullptr)
		return;

	const size_t PixelCount = static_cast<size_t>(Width) * static_cast<size_t>(Height) * (Type == FE_TEXTURE_TYPE::FE_TEXTURE_3D ? static_cast<size_t>(Depth) : 1);
	if (PixelCount == 0)
		return;

	// Interpret RawData with the same channel count and component type that UpdateRawData uploads it with.
	if (InternalFormat == GL_RED)
	{
		AccumulateMinMax(RawData, PixelCount, 1, MinValue, MaxValue);
	}
	else if (InternalFormat == GL_RGBA ||
			 InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ||
			 InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		AccumulateMinMax(RawData, PixelCount, 4, MinValue, MaxValue);
	}
	else if (InternalFormat == GL_R16)
	{
		AccumulateMinMax(reinterpret_cast<const unsigned short*>(RawData), PixelCount, 1, MinValue, MaxValue);
	}
	else if (InternalFormat == GL_RGBA16)
	{
		AccumulateMinMax(reinterpret_cast<const unsigned short*>(RawData), PixelCount, 4, MinValue, MaxValue);
	}
	else if (InternalFormat == GL_R32F)
	{
		AccumulateMinMax(reinterpret_cast<const float*>(RawData), PixelCount, 1, MinValue, MaxValue);
	}
	else if (InternalFormat == GL_RGBA32F)
	{
		AccumulateMinMax(reinterpret_cast<const float*>(RawData), PixelCount, 4, MinValue, MaxValue);
	}
	else if (InternalFormat == GL_R16F)
	{
		AccumulateMinMaxHalf(reinterpret_cast<const unsigned short*>(RawData), PixelCount, 1, MinValue, MaxValue);
	}
	else if (InternalFormat == GL_RGBA16F)
	{
		AccumulateMinMaxHalf(reinterpret_cast<const unsigned short*>(RawData), PixelCount, 4, MinValue, MaxValue);
	}
}