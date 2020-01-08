#pragma once

#include "../../../Renderer/FEShader.h"

static const char* const FESolidColorVS = R"(
#version 400 core

@In_Position@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 normal;
out vec3 FragPosition;

void main(void)
{
	normal = normalize(mat3(transpose(inverse(FEWorldMatrix))) * FENormal);
	FragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));

	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FESolidColorFS = R"(
#version 400 core

in vec3 normal;
in vec3 FragPosition;

uniform vec3 baseColor;
@LightPosition@
@LightColor@
@CameraPosition@
uniform float FEGamma;
uniform float FESpecularStrength;

void main(void)
{
	vec3 correctedBaseColor = pow(baseColor, vec3(FEGamma));
	vec3 lightDirection = normalize(FELightPosition - FragPosition);

	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * FELightColor;

	// specular part
	vec3 viewDirection = normalize(FECameraPosition - FragPosition);
	vec3 reflectedDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDirection, reflectedDirection), 0.0), 32);
	float specularStrength = FESpecularStrength;
	vec3 specular = specularStrength * specularFactor * FELightColor;

	vec3 ambientColor = vec3(0.1, 0.0f, 0.6f) * 0.3f;
	gl_FragColor = vec4(correctedBaseColor * (diffuseColor + ambientColor + specular), 1.0f);
}
)";

namespace FocalEngine 
{
	class FESolidColorShader : public FEShader
	{
	public:
		FESolidColorShader();
		~FESolidColorShader();
	private:
	};
}