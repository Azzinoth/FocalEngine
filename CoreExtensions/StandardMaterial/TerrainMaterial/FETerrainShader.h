#pragma once

#include "../../../Renderer/FEShader.h"

static const char* const FETerrainVS = R"(
#version 400 core

@In_Position@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 normal;
out vec3 fragPosition;

void main(void)
{
	normal = normalize(mat3(transpose(inverse(FEWorldMatrix))) * FENormal);
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));

	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FETerrainFS = R"(
#version 400 core

in vec3 normal;
in vec3 fragPosition;

uniform vec3 baseColor;
@CameraPosition@

void main(void)
{
	vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.2));
	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.15);
	vec3 diffuseColor = diffuseFactor * vec3(2.0, 2.0, 2.0);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 0.8f;

	gl_FragColor = vec4(baseColor * ambientColor * diffuseColor, 1.0f);
}
)";