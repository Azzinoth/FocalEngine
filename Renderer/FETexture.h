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
		static void GPUAllocateTeture(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data);
		static std::string textureInternalFormatToString(GLint internalFormat);

		FETexture(std::string Name);
		FETexture(int Width, int Height, std::string Name);
		FETexture(GLint InternalFormat, GLenum Format, int Width, int Height, std::string Name);
		~FETexture();

		GLuint getTextureID();

		std::string getFileName();

		virtual void bind(const unsigned int textureUnit = 0);
		virtual void unBind();

		GLint getInternalFormat();
		int getWidth();
		int getHeight();

		unsigned char* getRawData(size_t* rawDataSize = nullptr);
		void updateRawData(unsigned char* newRawData, size_t mipCount = 1);
	private:
		GLuint textureID = -1;
		void getNewGLTextureID();
		std::string fileName = "";
		bool hdr = false;

		int width = 0;
		int height = 0;
		GLint internalFormat;
		GLenum format;
		GLuint defaultTextureUnit = -1;

		FE_TEXTURE_MAG_FILTER magFilter = FE_LINEAR;
		bool mipEnabled = true;
		void addToOnDeleteCallBackList(std::string objectID);
		void eraseFromOnDeleteCallBackList(std::string objectID);

		static std::vector<GLuint> noDeletingList;
		static void addToNoDeletingList(GLuint textureID);
	};
}