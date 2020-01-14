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
out vec3 fragPosition;

void main(void)
{
	normal = normalize(mat3(transpose(inverse(FEWorldMatrix))) * FENormal);
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));

	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FESolidColorFS = R"(
#version 400 core

in vec3 normal;
in vec3 fragPosition;

uniform vec3 baseColor;
@CameraPosition@
uniform float FEGamma;
uniform float FESpecularStrength;

struct FELight
{
	int type;
	vec3 position;
	vec3 color;
	vec3 direction;
	float spotAngle;
	float spotAngleOuter;
};

#define MAX_LIGHTS 10
uniform FELight FElight[MAX_LIGHTS];

vec3 directionalLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor);
vec3 pointLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor);
vec3 spotLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor);

void main(void)
{
	vec3 baseColor = pow(baseColor, vec3(FEGamma));
	vec3 viewDirection = normalize(FECameraPosition - fragPosition);
	vec3 ambientColor = vec3(0.1, 0.0f, 0.6f) * 0.3f;

	gl_FragColor = vec4(baseColor * ambientColor, 0.0f);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (FElight[i].color.x == 0 && FElight[i].color.y == 0 && FElight[i].color.z == 0)
			continue;

		if (FElight[i].type == 0)
		{
			gl_FragColor += vec4(directionalLightColor(FElight[i], normal, fragPosition, viewDirection, baseColor), 1.0f);
		}
		else if (FElight[i].type == 1)
		{
			gl_FragColor += vec4(pointLightColor(FElight[i], normal, fragPosition, viewDirection, baseColor), 1.0f);
		}
		else if (FElight[i].type == 2)
		{
			gl_FragColor += vec4(spotLightColor(FElight[i], normal, fragPosition, viewDirection, baseColor), 1.0f);
		}
	}
}

vec3 directionalLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	vec3 lightDirection = normalize(-light.direction);

	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * light.color;
	// specular part
	vec3 reflectedDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDir, reflectedDirection), 0.0), 32);
	float specularStrength = 0.5;
	specularStrength = max(specularStrength, 0.0);

	vec3 specular = specularStrength * specularFactor * light.color;

	return (baseColor * (diffuseColor + specular));
}

vec3 pointLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	float distance = length(light.position - fragPosition);
	float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
	vec3 lightDirection = normalize(light.position - fragPosition);

	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * light.color;
	// specular part
	vec3 reflectedDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDir, reflectedDirection), 0.0), 32);
	float specularStrength = 0.5;
	specularStrength = max(specularStrength, 0.0);

	vec3 specular = specularStrength * specularFactor * light.color;

	return (baseColor * (diffuseColor * attenuation + specular * attenuation));
}

vec3 spotLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	vec3 lightDirection = normalize(light.position - fragPosition);
	float theta = dot(lightDirection, normalize(-light.direction));
	if(theta > light.spotAngleOuter)
	{
		float epsilon = light.spotAngle - light.spotAngleOuter;
		float intensity = clamp((theta - light.spotAngleOuter) / epsilon, 0.0, 1.0);
		float distance = length(light.position - fragPosition);
		float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

		// diffuse part
		float diffuseFactor = max(dot(normal, lightDirection), 0.0);
		vec3 diffuseColor = diffuseFactor * light.color;
		// specular part
		vec3 reflectedDirection = reflect(-lightDirection, normal);
		float specularFactor = pow(max(dot(viewDir, reflectedDirection), 0.0), 32);
		float specularStrength = 0.5;
		specularStrength = max(specularStrength, 0.0);

		vec3 specular = specularStrength * specularFactor * light.color;

		return (baseColor * (diffuseColor * attenuation * intensity + specular * attenuation * intensity));
	}

	return vec3(0.0, 0.0, 0.0);
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