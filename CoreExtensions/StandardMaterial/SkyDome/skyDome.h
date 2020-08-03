#pragma once

#include "../../../Renderer/FEShader.h"

static const char* const FEPhongVS = R"(
#version 450 core
@In_Position@
@In_UV@
@In_Normal@
@In_Tangent@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

#define MAX_LIGHTS 10
out VS_OUT
{
	vec2 UV;
	vec3 fragPosition;
	vec3 worldVertexPosition;
	mat3 TBN;
} vs_out;

void main(void)
{
	//vs_out.UV = FETexCoord;

	//vec3 T = normalize(vec3(FEWorldMatrix * vec4(FETangent, 0.0)));
	//vec3 N = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));
	// re-orthogonalize T with respect to N
	//T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	//vec3 B = cross(N, T);
    //vs_out.TBN = mat3(T, B, N);

	//vs_out.fragPosition = vec3(FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0));
	vs_out.fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	//vs_out.worldVertexPosition = (FEWorldMatrix * vec4(FEPosition, 1.0)).xyz;
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const FEPhongFS = R"(
#version 450 core

in VS_OUT
{
	vec2 UV;
	vec3 fragPosition;
	vec3 worldVertexPosition;
	mat3 TBN;
} FS_IN;

uniform float FEGamma;
uniform int debugFlag;

precision highp float;

#define PI 3.141592
#define iSteps 16
#define jSteps 8

vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}

vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {
    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    r = normalize(r);

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) return vec3(0,0,0);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0,0,0);
    vec3 totalMie = vec3(0,0,0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++) {

        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        // Increment the primary ray time.
        iTime += iStepSize;

    }

    // Calculate and return the final color.
    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

void main(void)
{
	vec3 uSunPos = vec3(0.0, 0.3, -1.0);
	vec3 color = atmosphere(
        normalize(FS_IN.fragPosition),           // normalized ray direction
        vec3(0,6372e3,0),               // ray origin
        uSunPos,                        // position of the sun
        22.0,                           // intensity of the sun
        6371e3,                         // radius of the planet in meters
        6471e3,                         // radius of the atmosphere in meters
        vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient
        21e-6,                          // Mie scattering coefficient
        8e3,                            // Rayleigh scale height
        1.2e3,                          // Mie scale height
        0.758                           // Mie preferred scattering direction
    );

	// Apply exposure.
    color = 1.0 - exp(-1.0 * color);

	gl_FragColor = vec4(color, 1.0f);
}

)";

static const char* const FEPhongAOFS = R"(
#version 450 core

#define MAX_LIGHTS 10
in VS_OUT
{
	vec2 UV;
	vec3 fragPosition;
	vec3 worldVertexPosition;
	mat3 TBN;
} FS_IN;

@Texture@ baseColorTexture;
@Texture@ normalsTexture;
@Texture@ AOTexture;
@Texture@ roughtnessMap;
@CameraPosition@
uniform float FEGamma;
uniform int debugFlag;

struct FELight
{
	vec3 typeAndAngles;
	vec3 position;
	vec3 color;
	vec3 direction;
	mat4 lightSpace;
	mat4 lightSpaceBig;
};

// is object receiving shadows.
@RECEVESHADOWS@
// adds cascade shadow maps, 4 cascades.
@CSM@

layout (set = 0, binding = 0, std140) uniform lightInfo
{
	FELight FElight[MAX_LIGHTS];
};

struct FEDirectionalLight
{
	vec3 position;
	vec3 color;
	vec3 direction;
	mat4 CSM0;
	mat4 CSM1;
	mat4 CSM2;
	mat4 CSM3;
	int activeCascades;
};

#define CSM_MAX_CASCADE 3

layout (set = 0, binding = 1, std140) uniform directionalLightInfo
{
	FEDirectionalLight directionalLight;
};

vec3 directionalLightColor(vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor);
vec3 pointLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor);
vec3 spotLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor);
float shadowCalculationCSM0(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);
float shadowCalculationCSM1(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);
float shadowCalculationCSM2(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);
float shadowCalculationCSM3(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

#define NUM_BLUR_TAPS 12

vec2 filterTaps[NUM_BLUR_TAPS] = vec2[] ( vec2(-0.326212, -0.405805), vec2(-0.840144, -0.07358),
										  vec2(-0.695914,  0.457137), vec2(-0.203345,  0.620716),
										  vec2( 0.96234,  -0.194983), vec2( 0.473434, -0.480026),
										  vec2( 0.519456,  0.767022), vec2( 0.185461, -0.893124),
										  vec2( 0.507431,  0.064425), vec2( 0.89642,   0.412458),
										  vec2(-0.32194,  -0.932615), vec2(-0.791559, -0.597705));

void main(void)
{
	// checking UV
	if (debugFlag == 3)
	{
		gl_FragColor = vec4(FS_IN.UV.x, FS_IN.UV.y, 0.0, 1.0);
		return;
	}
	// debug csm
	else if (debugFlag == 4)
	{
		// CSM0
		vec4 vertexInLightSpace = directionalLight.CSM0 * vec4(FS_IN.worldVertexPosition, 1.0);
		vec3 projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
		{
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
		}
		
		// CSM1
		if (directionalLight.activeCascades > 1)
		{
			vertexInLightSpace = directionalLight.CSM1 * vec4(FS_IN.worldVertexPosition, 1.0);
			projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
			projCoords = projCoords * 0.5 + 0.5;

			if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
			{
				gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
				return;
			}
		}

		// CSM2
		if (directionalLight.activeCascades > 2)
		{
			vertexInLightSpace = directionalLight.CSM2 * vec4(FS_IN.worldVertexPosition, 1.0);
			projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
			projCoords = projCoords * 0.5 + 0.5;

			if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
			{
				gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
				return;
			}
		}

		// CSM3
		if (directionalLight.activeCascades > 3)
		{
			vertexInLightSpace = directionalLight.CSM3 * vec4(FS_IN.worldVertexPosition, 1.0);
			projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
			projCoords = projCoords * 0.5 + 0.5;

			if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
			{
				gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
				return;
			}
		}
	}

	vec4 textureColor = texture(baseColorTexture, FS_IN.UV);
	if (textureColor.a < 0.05)
	{
		discard;
	}
		
	vec3 baseColor = pow(textureColor.rgb, vec3(FEGamma));
	vec3 viewDirection = normalize(FECameraPosition - FS_IN.fragPosition);
	vec3 ambientColor = (vec3(0.55f, 0.73f, 0.87f) * texture(AOTexture, FS_IN.UV).x) * 0.3f;

    vec3 normal = texture(normalsTexture, FS_IN.UV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(FS_IN.TBN * normal);

	gl_FragColor = vec4(baseColor * ambientColor, 0.0f);
	
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (FElight[i].color.x == 0 && FElight[i].color.y == 0 && FElight[i].color.z == 0)
			continue;

		if (FElight[i].typeAndAngles.x == 1)
		{
			gl_FragColor += vec4(pointLightColor(FElight[i], normal, FS_IN.fragPosition, viewDirection, baseColor), 1.0f);
		}
		else if (FElight[i].typeAndAngles.x == 2)
		{
			gl_FragColor += vec4(spotLightColor(FElight[i], normal, FS_IN.fragPosition, viewDirection, baseColor), 1.0f);
		}
	}

	gl_FragColor += vec4(directionalLightColor(normal, FS_IN.fragPosition, viewDirection, baseColor), 1.0f);
}

float shadowCalculationCSM0(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	float shadow = 0.0;
	
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(CSM0, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = 0.001;
	vec2 texelSize = 1.0 / textureSize(CSM0, 0);
	float fScale = 0.02f;
	
	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM0, projCoords.xy + filterTaps[i] * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	}
	shadow = shadow/NUM_BLUR_TAPS;

	return shadow;
}

float shadowCalculationCSM1(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	float shadow = 0.0;

	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(CSM1, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = 0.001;
	vec2 texelSize = 1.0 / textureSize(CSM1, 0);
	float fScale = 0.02f;
	
	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM1, projCoords.xy + filterTaps[i] * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	}
	shadow = shadow/NUM_BLUR_TAPS;

	return shadow;
}

float shadowCalculationCSM2(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	float shadow = 0.0;

	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(CSM2, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = 0.001;
	vec2 texelSize = 1.0 / textureSize(CSM2, 0);
	float fScale = 0.02f;
	
	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM2, projCoords.xy + filterTaps[i] * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	}
	shadow = shadow/NUM_BLUR_TAPS;

	return shadow;
}

float shadowCalculationCSM3(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	float shadow = 0.0;

	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(CSM3, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = 0.001;
	vec2 texelSize = 1.0 / textureSize(CSM3, 0);
	float fScale = 0.02f;
	
	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM3, projCoords.xy + filterTaps[i] * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	}
	shadow = shadow/NUM_BLUR_TAPS;

	return shadow;
}

vec3 directionalLightColor(vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	vec3 lightDirection = normalize(-directionalLight.direction.xyz);

	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * directionalLight.color.xyz;
	// specular part
	vec3 reflectedDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDir, reflectedDirection), 0.0), 32);
	
	float specularStrength = 0.5;
	specularStrength = (1.0 - texture(roughtnessMap, FS_IN.UV).r);
	
	specularStrength = max(specularStrength, 0.0);

	vec3 specular = specularStrength * specularFactor * directionalLight.color.xyz;
	
	if (FEReceiveShadows == 0)
		return (baseColor * (diffuseColor + specular));

	float shadow = 0.0;

	// first cascade
	vec4 vertexInLightSpace = directionalLight.CSM0 * vec4(FS_IN.worldVertexPosition, 1.0);
	vec3 projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
	{
		shadow = shadowCalculationCSM0(vertexInLightSpace, normal, lightDirection);
		return (baseColor * (diffuseColor + specular) * (1.0 - shadow));
	}
	
	// second cascade
	if (directionalLight.activeCascades > 1)
	{
		vertexInLightSpace = directionalLight.CSM1 * vec4(FS_IN.worldVertexPosition, 1.0);
		projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
		{
			shadow = shadowCalculationCSM1(vertexInLightSpace, normal, lightDirection);
			return (baseColor * (diffuseColor + specular) * (1.0 - shadow));
		}
	}

	// third cascade
	if (directionalLight.activeCascades > 2)
	{
		vertexInLightSpace = directionalLight.CSM2 * vec4(FS_IN.worldVertexPosition, 1.0);
		projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
		{
			shadow = shadowCalculationCSM2(vertexInLightSpace, normal, lightDirection);
			return (baseColor * (diffuseColor + specular) * (1.0 - shadow));
		}
	}

	// fourth cascade
	if (directionalLight.activeCascades > 3)
	{
		vertexInLightSpace = directionalLight.CSM3 * vec4(FS_IN.worldVertexPosition, 1.0);
		projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
		{
			shadow = shadowCalculationCSM3(vertexInLightSpace, normal, lightDirection);
			return (baseColor * (diffuseColor + specular) * (1.0 - shadow));
		}
	}

	return (baseColor * (diffuseColor + specular) * (1.0 - shadow));
}

vec3 pointLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	float distance = length(light.position.xyz - fragPosition);
	float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
	vec3 lightDirection = normalize(light.position.xyz - fragPosition);

	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * light.color.xyz;
	// specular part
	vec3 reflectedDirection = reflect(-lightDirection, normal);
	float specularFactor = pow(max(dot(viewDir, reflectedDirection), 0.0), 32);
	float specularStrength = 0.5;
	specularStrength = max(specularStrength, 0.0);

	vec3 specular = specularStrength * specularFactor * light.color.xyz;

	return (baseColor * (diffuseColor * attenuation + specular * attenuation));
}

vec3 spotLightColor(FELight light, vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	vec3 lightDirection = normalize(light.position.xyz - fragPosition);
	float theta = dot(lightDirection, normalize(-light.direction.xyz));
	if(theta > light.typeAndAngles.z)
	{
		float epsilon = light.typeAndAngles.y - light.typeAndAngles.z;
		float intensity = clamp((theta - light.typeAndAngles.z) / epsilon, 0.0, 1.0);
		float distance = length(light.position.xyz - fragPosition);
		float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

		// diffuse part
		float diffuseFactor = max(dot(normal, lightDirection), 0.0);
		vec3 diffuseColor = diffuseFactor * light.color.xyz;
		// specular part
		vec3 reflectedDirection = reflect(-lightDirection, normal);
		float specularFactor = pow(max(dot(viewDir, reflectedDirection), 0.0), 32);
		float specularStrength = 0.5;
		specularStrength = max(specularStrength, 0.0);

		vec3 specular = specularStrength * specularFactor * light.color.xyz;

		return (baseColor * (diffuseColor * attenuation * intensity + specular * attenuation * intensity));
	}

	return vec3(0.0, 0.0, 0.0);
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