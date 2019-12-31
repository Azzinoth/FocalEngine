#pragma once

#include "../../../Renderer/FEShader.h"

static const char* const FEPhongVS = R"(
#version 400 core

@In_Position@
@In_UV@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec2 UV;
out vec3 normal;
out vec3 FragPosition;

void main(void)
{
	UV = FETexCoord;
	normal = normalize(mat3(transpose(inverse(FEWorldMatrix))) * FENormal);
	FragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));

	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FEPhongFS = R"(
#version 400 core

in vec2 UV;
in vec3 normal;
in vec3 FragPosition;

uniform sampler2D baseColorTexture;
@LightPosition@
@LightColor@
@CameraPosition@

void main(void)
{
	vec3 baseColor = texture(baseColorTexture, UV, -1.0).xyz;
	vec3 lightDirection = normalize(FELightPosition - FragPosition);

	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * FELightColor;

	// specular part
	vec3 viewDirection = normalize(FECameraPosition - FragPosition);
	vec3 reflectedDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDirection, reflectedDirection), 0.0), 32);
	float specularStrength = 0.5;
	vec3 specular = specularStrength * specularFactor * FELightColor;

	vec3 ambientColor = vec3(0.1, 0.0f, 0.6f) * 0.3f;
	gl_FragColor = vec4(baseColor * (diffuseColor + ambientColor + specular), 1.0f);
}
)";

namespace FocalEngine 
{
	class FEPhongShader : public FEShader
	{
	public:
		FEPhongShader();
		~FEPhongShader();

	private:
	};
}