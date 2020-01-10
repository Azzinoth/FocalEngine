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
@Texture@ NormalsTexture;
@LightPosition@
@LightColor@
@CameraPosition@
uniform float FEGamma;

void main(void)
{
	vec3 baseColor = pow(texture(baseColorTexture, UV).rgb, vec3(FEGamma));
	vec3 lightDirection = normalize(FELightPosition - FragPosition);

    vec3 normal = texture(NormalsTexture, UV).rgb;
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