#pragma once

#include "../../Renderer/FEShader.h"

static const char* const FEStandardVS = R"(
#version 400 core

in vec3 vPos;
@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

void main(void)
{
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(vPos, 1.0);
}
)";

static const char* const FEStandardFS = R"(
#version 400 core

void main(void)
{
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

namespace FocalEngine 
{
	class FEStandardShader : public FEShader
	{
	public:
		FEStandardShader();
		~FEStandardShader();

	private:

	};
}