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
		friend FERenderer;
	public:
		~FEFramebuffer();

		void bind();
		void unBind();

		FETexture* getColorAttachment(size_t index = 0);
		FETexture* getDepthAttachment();
		FETexture* getStencilAttachment();

		void setColorAttachment(FETexture* newTexture, size_t index = 0);
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
		
		std::vector<FETexture*> colorAttachments;
		FETexture* depthAttachment = nullptr;
		FETexture* stencilAttachment = nullptr;

		void attachTexture(GLenum attachment, GLenum textarget, FETexture* texture);
	};
}