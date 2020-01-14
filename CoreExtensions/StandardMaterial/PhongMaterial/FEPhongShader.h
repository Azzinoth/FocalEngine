#pragma once

#include "../../../Renderer/FEShader.h"

static const char* const FEPhongVS = R"(
#version 400 core
@In_Position@
@In_UV@
@In_Normal@
@In_Tangent@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@
out vec2 UV;
out vec3 fragPosition;
out mat3 TBN;

void main(void)
{
	UV = FETexCoord;

	vec3 T = normalize(vec3(FEWorldMatrix * vec4(FETangent, 0.0)));
	vec3 N = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FEPhongFS = R"(
#version 400 core
in vec2 UV;
in vec3 fragPosition;
in mat3 TBN;

@Texture@ baseColorTexture;
@Texture@ normalsTexture;
@CameraPosition@
uniform float FEGamma;

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
	vec3 baseColor = pow(texture(baseColorTexture, UV).rgb, vec3(FEGamma));
	vec3 viewDirection = normalize(FECameraPosition - fragPosition);
	vec3 ambientColor = vec3(0.1, 0.0f, 0.6f) * 0.3f;

    vec3 normal = texture(normalsTexture, UV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

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

//vec3 lightDirection;
//float distance;
//float attenuation = 1.0;
//float intensity = 1.0;
//
//if (FELightType == 0)
//{
//	lightDirection = normalize(-FELightDirection);
//}
//else if (FELightType == 1)
//{
//	attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
//	distance = length(FELightPosition - fragPosition);
//	lightDirection = normalize(FELightPosition - fragPosition);
//}
//else if (FELightType == 2)
//{
//	lightDirection = normalize(FELightPosition - fragPosition);
//	float theta = dot(lightDirection, normalize(-FELightDirection));
//	if (theta > FELightSpotAngleOuter)
//	{
//		float epsilon = FELightSpotAngle - FELightSpotAngleOuter;
//		intensity = clamp((theta - FELightSpotAngleOuter) / epsilon, 0.0, 1.0);
//		attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
//	}
//	else
//	{
//		attenuation = 0.0;
//	}
//}

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