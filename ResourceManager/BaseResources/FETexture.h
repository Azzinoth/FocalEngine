#pragma once

#include "../Core/FEObject.h"

namespace FocalEngine
{
	enum class FE_TEXTURE_TYPE
	{
		FE_TEXTURE_NONE = 0,
		FE_TEXTURE_1D = 1,
		FE_TEXTURE_2D = 2,
		FE_TEXTURE_3D = 3,
		FE_TEXTURE_CUBE = 4
	};

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
		static std::string TextureInternalFormatToString(GLint InternalFormat);

		FETexture(std::string Name, FE_TEXTURE_TYPE TextureType = FE_TEXTURE_TYPE::FE_TEXTURE_2D);
		FETexture(int Width, int Height, std::string Name, FE_TEXTURE_TYPE TextureType = FE_TEXTURE_TYPE::FE_TEXTURE_2D);
		FETexture(GLint InternalFormat, GLenum Format, int Width, int Height, std::string Name, FE_TEXTURE_TYPE TextureType = FE_TEXTURE_TYPE::FE_TEXTURE_2D);
		~FETexture();

		FE_TEXTURE_TYPE GetType() const;
		GLuint GetTextureID();

		std::string GetFileName();

		virtual void Bind(const unsigned int TextureUnit = 0);
		virtual void UnBind();

		GLint GetInternalFormat();
		int GetWidth();
		int GetHeight();
		int GetDepth();

		glm::vec4 GetMinValue();
		glm::vec4 GetMaxValue();

		unsigned char* GetRawData(size_t* RawDataSize = nullptr);
		void UpdateRawData(unsigned char* NewRawData, size_t MipmapCount = 1);

		void ForceUpdateMinMaxValues();
	private:
		FE_TEXTURE_TYPE Type = FE_TEXTURE_TYPE::FE_TEXTURE_NONE;
		GLuint TextureID = -1;
		void GetNewGlTextureID();
		std::string FileName;
		bool bHDR = false;

		int Width = 0;
		int Height = 0;
		int Depth = 1;
		GLint InternalFormat;
		GLenum Format;
		GLuint DefaultTextureUnit = -1;

		glm::vec4 MinValue = glm::vec4(std::numeric_limits<float>::max());
		glm::vec4 MaxValue = glm::vec4(-std::numeric_limits<float>::max());
		void UpdateMinMaxValues(const unsigned char* RawData);

		FE_TEXTURE_MAG_FILTER MagFilter = FE_LINEAR;
		bool bMipmapEnabled = true;
		void AddToOnDeleteCallBackList(std::string ObjectID);
		void EraseFromOnDeleteCallBackList(std::string ObjectID);

		static std::vector<GLuint> PreventAutoDeletionList;
		static void MarkAsPersistent(GLuint TextureID);
	};
}