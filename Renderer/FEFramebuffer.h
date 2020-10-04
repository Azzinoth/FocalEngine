#pragma once

#include "FEMaterial.h"

namespace FocalEngine
{
	enum FE_FRAMEBUFFER_ATTACHMENT
	{
		FE_COLOR_ATTACHMENT = 1,
		FE_DEPTH_ATTACHMENT = 2,
		FE_STENCIL_ATTACHMENT = 4,
	};

	class FEResourceManager;

	class FEFramebuffer
	{
		friend FEResourceManager;
	public:
		~FEFramebuffer();

		void bind();
		void unBind();

		FETexture* getColorAttachment();
		FETexture* getDepthAttachment();
		FETexture* getStencilAttachment();

		void setColorAttachment(FETexture* newTexture);
		void setDepthAttachment(FETexture* newTexture);
		void setStencilAttachment(FETexture* newTexture);

		int getWidth();
		int getHeight();
	private:
		FEFramebuffer(/*int attachments, int Width, int Height, bool HDR = true*/);

		GLuint fbo = -1;
		bool binded = false;

		int width = 0;
		int height = 0;

		FETexture* colorAttachment = nullptr; // std::make_unique( std::unique_ptr<FETexture*>
		FETexture* depthAttachment = nullptr;
		FETexture* stencilAttachment = nullptr;

		void attachTexture(GLenum attachment, GLenum textarget, FETexture* texture);
	};
}