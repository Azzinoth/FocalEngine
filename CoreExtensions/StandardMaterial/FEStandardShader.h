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

uniform vec3 baseColor;
uniform vec3 secondBaseColor;

void main(void)
{
	gl_FragColor = mix(vec4(baseColor, 1.0), vec4(secondBaseColor, 1.0), 0.5);
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
		glm::vec3 baseColor;
		
	};
}