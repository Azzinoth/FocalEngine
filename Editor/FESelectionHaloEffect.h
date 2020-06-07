#pragma once

static const char* const HaloDrawObjectVS = R"(
#version 400 core

@In_Position@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 fragPosition;

void main(void)
{
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const HaloDrawObjectFS = R"(
#version 400 core

in vec3 fragPosition;
@CameraPosition@

void main(void)
{
	vec3 viewDirection = normalize(FECameraPosition - fragPosition);
	gl_FragColor = vec4(1.0f, 0.25f, 0.0f, 1.0f);
}

)";

static const char* const HaloFinalVS = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* const HaloFinalFS = R"(
#version 400 core

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

