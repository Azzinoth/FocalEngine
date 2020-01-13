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
out vec3 FragPosition;
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

	FragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FEPhongFS = R"(
#version 400 core
in vec2 UV;
in vec3 FragPosition;
in mat3 TBN;

@Texture@ baseColorTexture;
@Texture@ normalsTexture;
//@Texture@ roughnessTexture;
@LightPosition@
@LightColor@
@CameraPosition@
uniform float FEGamma;
uniform int LightType;
uniform vec3 FELightDirection;
uniform float LightSpotAngle;
uniform float LightSpotAngleOuter;

void main(void)
{
	vec3 baseColor = pow(texture(baseColorTexture, UV).rgb, vec3(FEGamma));

	vec3 lightDirection;
	float distance;
	float attenuation = 1.0;
	float intensity = 1.0;
	if (LightType == 0)
	{
		lightDirection = normalize(-FELightPosition);
	}
	else if (LightType == 1)
	{
		attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
		distance = length(FELightPosition - FragPosition);
		lightDirection = normalize(FELightPosition - FragPosition);
	}
	else if (LightType == 2)
	{
		lightDirection = normalize(FELightPosition - FragPosition);
		float theta = dot(lightDirection, normalize(-FELightDirection));
		if(theta > LightSpotAngle)
		{
			//float epsilon = LightSpotAngle - LightSpotAngleOuter;
			//intensity = clamp((theta - LightSpotAngleOuter) / epsilon, 0.0, 1.0);
			float epsilon = LightSpotAngle - LightSpotAngleOuter;
			intensity = clamp((LightSpotAngleOuter - theta) / epsilon, 0.0, 1.0);


			attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
		}
		else
		{
			attenuation = 0.0;
		}
	}
	

    vec3 normal = texture(normalsTexture, UV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * FELightColor;
	// specular part
	vec3 viewDirection = normalize(FECameraPosition - FragPosition);
	vec3 reflectedDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDirection, reflectedDirection), 0.0), 32);
	float specularStrength = 0.5;
	//vec3 specularMap = texture(roughnessTexture, UV).rgb;
	//float specularStrength = 1.0 - (specularMap.r + specularMap.g + specularMap.b);
	//float specularStrength = texture(roughnessTexture, UV).r;
	specularStrength = max(specularStrength, 0.0);

	vec3 specular = specularStrength * specularFactor * FELightColor;
	vec3 ambientColor = vec3(0.1, 0.0f, 0.6f) * 0.3f;

	diffuseColor *= attenuation;
	specular *= attenuation;

	diffuseColor *= intensity;
	specular *= intensity;

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