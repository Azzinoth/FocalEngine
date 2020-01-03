#pragma once

#include "FEMaterial.h"

namespace FocalEngine
{
	enum FE_FRAMEBUFFER_ATTACHMENT
	{
		FE_COLOR_ATTACHMENT = 0,
		FE_DEPTH_ATTACHMENT = 1,
		FE_STENCIL_ATTACHMENT = 2,
	};

	class FEFramebuffer
	{
	public:
		FEFramebuffer(int attachments, int Width, int Height);
		~FEFramebuffer();

		void bind();
		void unBind();

		FETexture* getColorAttachment();
		FETexture* getDepthAttachment();
		FETexture* getStencilAttachment();
	private:
		GLuint fbo = -1;

		int width = 0;
		int height = 0;

		FETexture* colorAttachment = nullptr;
		FETexture* depthAttachment = nullptr;
		FETexture* stencilAttachment = nullptr;
	};
}