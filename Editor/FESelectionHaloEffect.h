#pragma once

static const char* const HaloDrawObjectVS = R"(
#version 450 core

@In_Position@
@In_UV@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 fragPosition;
out vec2 UV;

void main(void)
{
	UV = FETexCoord;
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const HaloDrawObjectFS = R"(
#version 450 core

in vec3 fragPosition;
in vec2 UV;

@Texture@ objectTexture;
@CameraPosition@

void main(void)
{
	vec4 textureColor = texture(objectTexture, UV);
	if (textureColor.a < 0.05)
	{
		discard;
	}

	gl_FragColor = vec4(1.0f, 0.25f, 0.0f, 1.0f);
}

)";

static const char* const HaloFinalVS = R"(
#version 450 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* const HaloFinalFS = R"(
#version 450 core

in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ haloBluredTexture;
@Texture@ haloMaskTexture;

void main(void)
{
	if (texture(haloMaskTexture, textureCoords).a < 1)
	{
		gl_FragColor = texture(sceneTexture, textureCoords) + texture(haloBluredTexture, textureCoords);
	}
	else
	{
		gl_FragColor = texture(sceneTexture, textureCoords);
	}
}

)";

