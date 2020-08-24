#pragma once

#include "../../../Renderer/FEShader.h"

static const char* const FEPixelAccurateSelectionVS = R"(
#version 450 core

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

static const char* const FEPixelAccurateSelectionFS = R"(
#version 450 core

in vec3 fragPosition;
in vec2 UV;

uniform vec3 baseColor;
@Texture@ objectTexture;
@CameraPosition@

void main(void)
{
	vec4 textureColor = texture(objectTexture, UV);
	if (textureColor.a < 0.05)
	{
		discard;
	}

    gl_FragColor = vec4(baseColor, 1.0f);
}
)";