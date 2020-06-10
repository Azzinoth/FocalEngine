#pragma once

#include "../../../Renderer/FEShader.h"

static const char* const FEShadowMapVS = R"(
#version 400 core
@In_Position@
@In_UV@

out vec2 UV;

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@
out vec3 fragPosition;

void main(void)
{
	UV = FETexCoord;
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FEShadowMapFS = R"(
#version 400 core
in vec3 fragPosition;
in vec2 UV;

@Texture@ baseColorTexture;
@CameraPosition@

void main(void)
{
	vec4 textureColor = texture(baseColorTexture, UV);
	if (textureColor.a < 0.05)
	{
		discard;
	}

	gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

)";

namespace FocalEngine 
{
	class FEShadowMap : public FEShader
	{
	public:
		FEShadowMap();
		~FEShadowMap();

	private:
	};
}