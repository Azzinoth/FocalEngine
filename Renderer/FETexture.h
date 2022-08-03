#pragma once

#include "../SubSystems/FEObject.h"

namespace FocalEngine
{
	enum FE_TEXTURE_MAG_FILTER
	{
		FE_NEAREST = 0,
		FE_LINEAR = 1,
	};

	class FEResourceManager;
	class FERenderer;
	class FEPostProcess;
	class FEFramebuffer;

	class FETexture : public FEObject
	{
		friend FEResourceManager;
		friend FERenderer;
		friend FEPostProcess;
		friend FEFramebuffer;
	public:
		static void GPUAllocateTeture(GLenum Target, GLint Level, GLint Internalformat, GLsizei Width, GLsizei Height, GLint Border, GLenum Format, GLenum Type, const void* Data);
		static std::string TextureInternalFormatToString(GLint InternalFormat);

		FETexture(std::string Name);
		FETexture(int Width, int Height, std::string Name);
		FETexture(GLint InternalFormat, GLenum Format, int Width, int Height, std::string Name);
		~FETexture();

		GLuint GetTextureID();

		std::string GetFileName();

		virtual void Bind(const unsigned int TextureUnit = 0);
		virtual void UnBind();

		GLint GetInternalFormat();
		int GetWidth();
		int GetHeight();

		unsigned char* GetRawData(size_t* RawDataSize = nullptr);
		void UpdateRawData(unsigned char* NewRawData, size_t MipCount = 1);
	private:
		GLuint TextureID = -1;
		void GetNewGlTextureID();
		std::string FileName;
		bool bHDR = false;

		int Width = 0;
		int Height = 0;
		GLint InternalFormat;
		GLenum Format;
		GLuint DefaultTextureUnit = -1;

		FE_TEXTURE_MAG_FILTER MagFilter = FE_LINEAR;
		bool MipEnabled = true;
		void AddToOnDeleteCallBackList(std::string ObjectID);
		void EraseFromOnDeleteCallBackList(std::string ObjectID);

		static std::vector<GLuint> NoDeletingList;
		static void AddToNoDeletingList(GLuint TextureID);
	};
}