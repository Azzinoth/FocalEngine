#pragma once

#include "../../../Renderer/FEShader.h"

static char* FETerrainVS = R"(
#version 420 core

out vec2 vertex_UV;

void main(void)
{
    const vec4 vertices[] = vec4[](vec4(-0.5, 0.0, -0.5, 1.0),
                                   vec4( 0.5, 0.0, -0.5, 1.0),
                                   vec4(-0.5, 0.0,  0.5, 1.0),
                                   vec4( 0.5, 0.0,  0.5, 1.0));

    int x = gl_InstanceID & 63;
    int y = gl_InstanceID >> 6;
    vec2 offs = vec2(x, y);
 
	vertex_UV = (vertices[gl_VertexID].xz + offs + vec2(0.5)) / 64.0;
    gl_Position = vertices[gl_VertexID] + vec4(float(x - 32), 0.0, float(y - 32), 0.0);
}
)";

static char* FETerrainTCS = R"(
#version 420 core

layout (vertices = 4) out;

@PVMMatrix@
@CameraPosition@

@Texture@ heightMap;
uniform float hightScale;
uniform float scaleFactor;
uniform float LODlevel;
uniform vec2 hightMapShift;

in vec2 vertex_UV[];
out vec2 TC_UV[];

float height(float u, float v)
{
	return (texture(heightMap,(vec2(u,v) + hightMapShift) / scaleFactor).r * hightScale);
}

// frustum_edge should be 1, but I gave additional 10% for less aggressive culling. 
#define f 1.1
// Checks if a segment is at least partially inside the frustum
bool segmentInFrustum(vec4 p1, vec4 p2)
{
	p1 = p1 / p1.w;
	p2 = p2 / p2.w;

	if (((p1.x < -f || p1.x > f) || (p1.y < -f || p1.y > f) || (p1.z < -f || p1.z > f)) &&
	    ((p2.x < -f || p2.x > f) || (p2.y < -f || p2.y > f) || (p2.z < -f || p2.z > f)))
		return false;
	else
		return true;
}

void main(void)
{
	if (gl_InvocationID == 0)
    {
		vec4 patchPointsPosition[4];
		patchPointsPosition[0] = vec4(gl_in[0].gl_Position.x, height(vertex_UV[0].x, vertex_UV[0].y), gl_in[0].gl_Position.z, 1.0);
		patchPointsPosition[1] = vec4(gl_in[1].gl_Position.x, height(vertex_UV[1].x, vertex_UV[1].y), gl_in[1].gl_Position.z, 1.0);
		patchPointsPosition[2] = vec4(gl_in[2].gl_Position.x, height(vertex_UV[2].x, vertex_UV[2].y), gl_in[2].gl_Position.z, 1.0);
		patchPointsPosition[3] = vec4(gl_in[3].gl_Position.x, height(vertex_UV[3].x, vertex_UV[3].y), gl_in[3].gl_Position.z, 1.0);
		
		patchPointsPosition[0] = FEPVMMatrix * patchPointsPosition[0];
		patchPointsPosition[1] = FEPVMMatrix * patchPointsPosition[1];
		patchPointsPosition[2] = FEPVMMatrix * patchPointsPosition[2];
		patchPointsPosition[3] = FEPVMMatrix * patchPointsPosition[3];

		if (segmentInFrustum(patchPointsPosition[0], patchPointsPosition[0+1]) ||
			segmentInFrustum(patchPointsPosition[0], patchPointsPosition[0+2]) ||
			segmentInFrustum(patchPointsPosition[0+2], patchPointsPosition[0+3]) ||
			segmentInFrustum(patchPointsPosition[0+3], patchPointsPosition[0+1]))
		{
			vec4 p0 = FEPVMMatrix * gl_in[0].gl_Position;
			vec4 p1 = FEPVMMatrix * gl_in[1].gl_Position;
			vec4 p2 = FEPVMMatrix * gl_in[2].gl_Position;
			vec4 p3 = FEPVMMatrix * gl_in[3].gl_Position;

			p0 /= p0.w;
			p1 /= p1.w;
			p2 /= p2.w;
			p3 /= p3.w;
			
			float l0 = length(p2.xy - p0.xy) * LODlevel + 1.0;
			float l1 = length(p3.xy - p2.xy) * LODlevel + 1.0;
			float l2 = length(p3.xy - p1.xy) * LODlevel + 1.0;
			float l3 = length(p1.xy - p0.xy) * LODlevel + 1.0;

			gl_TessLevelOuter[0] = l0;
			gl_TessLevelOuter[1] = l1;
			gl_TessLevelOuter[2] = l2;
			gl_TessLevelOuter[3] = l3;
			gl_TessLevelInner[0] = min(l1, l3);
			gl_TessLevelInner[1] = min(l0, l2);
		}
		else
		{
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelOuter[3] = 0;

			gl_TessLevelInner[0] = 0;
			gl_TessLevelInner[1] = 0;
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	TC_UV[gl_InvocationID] = (vertex_UV[gl_InvocationID] + hightMapShift) / scaleFactor;
}
)";

static char* FETerrainTES = R"(
#version 420 core

layout (quads, fractional_even_spacing) in;

uniform float hightScale;
//uniform float scaleFactor;
//uniform vec2 hightMapShift;

@Texture@ heightMap;

in vec2 TC_UV[];
out vec2 TES_UV;
out vec3 TES_normal;

float height(float u, float v)
{
	return (texture(heightMap, vec2(u,v)).r * hightScale);
}

void main(void)
{
    vec2 tc1 = mix(TC_UV[0], TC_UV[1], gl_TessCoord.x);
    vec2 tc2 = mix(TC_UV[2], TC_UV[3], gl_TessCoord.x);
    TES_UV = mix(tc2, tc1, gl_TessCoord.y);// / scaleFactor;
	
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 p = mix(p2, p1, gl_TessCoord.y);
	p.y += texture(heightMap, TES_UV).r * hightScale;

	// calculate normal
	float gridSpacing = 1.0;
	float delta =  1.0 / (textureSize(heightMap,0).x);//* scaleFactor

	vec3 deltaX = vec3( 2.0 * gridSpacing,
						height(TES_UV.x + delta, TES_UV.y) - height(TES_UV.x - delta, TES_UV.y) , 
						0.0) ;
				
	vec3 deltaZ = vec3( 0.0, 
						height(TES_UV.x, TES_UV.y + delta) - height(TES_UV.x, TES_UV.y - delta) , 
						2.0 * gridSpacing) ;
	
	TES_normal = normalize(cross(deltaZ, deltaX));
    gl_Position = p;
}
)";

static char* FETerrainGS = R"(
#version 420 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

@WorldMatrix@
@PVMMatrix@

in vec2 TES_UV[];
in vec3 TES_normal[];

out GS_OUT
{
	vec2 UV;
	vec3 fragPosition;
	vec3 worldVertexPosition;
	mat3 TBN;
} gs_out;

mat3 getTBN(vec3 tangent, vec3 normal)
{
	vec3 T = normalize(vec3(FEWorldMatrix * vec4(tangent, 0.0)));
	vec3 N = normalize(vec3(FEWorldMatrix * vec4(normal, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);

	return mat3(T, B, N);
}

void main(void)
{
	vec3 q1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 q2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec2 uv0 = TES_UV[0];
	vec2 uv1 = TES_UV[1];
	vec2 uv2 = TES_UV[2];

	float t1 = uv1.y - uv0.y;
	float t2 = uv2.y - uv0.y;
	vec3 tangent = t2 * q1 - t1 * q2;


	gl_Position = FEPVMMatrix * gl_in[0].gl_Position;
	gs_out.fragPosition = vec3(FEWorldMatrix * vec4(gl_in[0].gl_Position.xyz, 1.0));
	gs_out.worldVertexPosition = vec3(FEWorldMatrix * vec4(gl_in[0].gl_Position.xyz, 1.0));
	gs_out.UV = TES_UV[0];
	gs_out.TBN = getTBN(tangent, TES_normal[0]);
	EmitVertex();

	gl_Position = FEPVMMatrix * gl_in[1].gl_Position;
	gs_out.fragPosition = vec3(FEWorldMatrix * vec4(gl_in[1].gl_Position.xyz, 1.0));
	gs_out.worldVertexPosition = vec3(FEWorldMatrix * vec4(gl_in[1].gl_Position.xyz, 1.0));
	gs_out.UV = TES_UV[1];
	gs_out.TBN = getTBN(tangent, TES_normal[1]);
	EmitVertex();

	gl_Position = FEPVMMatrix * gl_in[2].gl_Position;
	gs_out.fragPosition = vec3(FEWorldMatrix * vec4(gl_in[2].gl_Position.xyz, 1.0));
	gs_out.worldVertexPosition = vec3(FEWorldMatrix * vec4(gl_in[2].gl_Position.xyz, 1.0));
	gs_out.UV = TES_UV[2];
	gs_out.TBN = getTBN(tangent, TES_normal[2]);
	EmitVertex();

    EndPrimitive();
}
)";

static char* FETerrainFS = R"(
#version 450 core

uniform float hightScale;
uniform float scaleFactor;
uniform vec2 tileMult;
uniform float drawingToShadowMap;

@Texture@ heightMap;
@Texture@ albedoMap;
@Texture@ normalMap;
@Texture@ AOMap;
uniform float FEAO;
@Texture@ roughtnessMap;
uniform float FERoughtness;
@Texture@ metalnessMap;
uniform float FEMetalness;

#define MAX_LIGHTS 10
in GS_OUT
{
	vec2 UV;
	vec3 fragPosition;
	vec3 worldVertexPosition;
	mat3 TBN;
} FS_IN;

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

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------

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
	if (drawingToShadowMap > 0)
	{
		gl_FragColor = vec4(1.0);
		return;
	}

	vec2 tiledUV = vec2(FS_IN.UV.x * tileMult[0], FS_IN.UV.y * tileMult[1]);

	vec4 textureColor = texture(albedoMap, tiledUV);
	if (textureColor.a < 0.05)
	{
		discard;
	}
		
	vec3 baseColor = pow(textureColor.rgb, vec3(FEGamma));
	vec3 viewDirection = normalize(FECameraPosition - FS_IN.fragPosition);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 0.6f;

    vec3 normal = texture(normalMap, tiledUV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(FS_IN.TBN * normal);
	
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

	float ao = FEAO;
	if (FEAO == -1)
		ao = texture(AOMap, tiledUV).r;

	gl_FragColor = vec4(directionalLightColor(normal, FS_IN.fragPosition, viewDirection, baseColor), 1.0f);
	gl_FragColor += vec4(baseColor * ambientColor * 0.3 * ao, 1.0f);
}

// Produces cheap but low-quality white noise, nothing special
float quick_hash(vec2 pos)
{
	return fract(sin(dot(pos * 19.19, vec2(49.5791, 97.413))) * 49831.189237);
}

float shadowCalculationCSM0(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	float shadow = 0.0;
	
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(CSM0, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float minBias = 0.0005 * 2.0;
	float baseBias = 0.001 * 2.0;
	float bias = max(baseBias * (1.0 - dot(normal, lightDir)), minBias);
	vec2 texelSize = 1.0 / textureSize(CSM0, 0);
	
	mat2 disk_rotation;
	{
		float r = quick_hash(gl_FragCoord.xy) * 2.0 * PI;
		float sr = sin(r);
		float cr = cos(r);
		disk_rotation = mat2(vec2(cr, -sr), vec2(sr, cr));
	}

	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM0, projCoords.xy + disk_rotation * filterTaps[i] * texelSize).r;
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
	float minBias = 0.0005 * 2.0;
	float baseBias = 0.001 * 2.0;
	float bias = max(baseBias * (1.0 - dot(normal, lightDir)), minBias);
	vec2 texelSize = 1.0 / textureSize(CSM1, 0);
	
	mat2 disk_rotation;
	{
		float r = quick_hash(gl_FragCoord.xy) * 2.0 * PI;
		float sr = sin(r);
		float cr = cos(r);
		disk_rotation = mat2(vec2(cr, -sr), vec2(sr, cr));
	}

	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM1, projCoords.xy + disk_rotation * filterTaps[i] * texelSize).r;
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
	float minBias = 0.0005 * 2.0;
	float baseBias = 0.001 * 2.0;
	float bias = max(baseBias * (1.0 - dot(normal, lightDir)), minBias);
	vec2 texelSize = 1.0 / textureSize(CSM2, 0);
	
	mat2 disk_rotation;
	{
		float r = quick_hash(gl_FragCoord.xy) * 2.0 * PI;
		float sr = sin(r);
		float cr = cos(r);
		disk_rotation = mat2(vec2(cr, -sr), vec2(sr, cr));
	}

	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM2, projCoords.xy + disk_rotation * filterTaps[i] * texelSize).r;
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
	float minBias = 0.0005 * 2.0;
	float baseBias = 0.001 * 2.0;
	float bias = max(baseBias * (1.0 - dot(normal, lightDir)), minBias);
	vec2 texelSize = 1.0 / textureSize(CSM3, 0);
	
	mat2 disk_rotation;
	{
		float r = quick_hash(gl_FragCoord.xy) * 2.0 * PI;
		float sr = sin(r);
		float cr = cos(r);
		disk_rotation = mat2(vec2(cr, -sr), vec2(sr, cr));
	}

	for(int i = 0; i < NUM_BLUR_TAPS; i++)
	{
		float pcfDepth = texture(CSM3, projCoords.xy + disk_rotation * filterTaps[i] * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	}
	shadow = shadow/NUM_BLUR_TAPS;

	return shadow;
}

vec3 directionalLightColor(vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	vec2 tiledUV = vec2(FS_IN.UV.x * tileMult[0], FS_IN.UV.y * tileMult[1]);
	vec3 lightDirection = normalize(-directionalLight.direction.xyz);

	vec3 albedo = baseColor;
	float metallic = FEMetalness;
	if (FEMetalness == -1)
		metallic = texture(metalnessMap, tiledUV).r;

	float roughness = FEMetalness;
	if (FERoughtness == -1)
		roughness = texture(roughtnessMap, tiledUV).r;

    vec3 N = normal;
    vec3 V = viewDir;
	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

	// calculate per-light radiance
    vec3 L = lightDirection;
    vec3 H = normalize(V + L);
    vec3 radiance = directionalLight.color.xyz;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
    vec3 nominator    = NDF * G * F; 
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);   

    // add to outgoing radiance Lo
    Lo = (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

	if (FEReceiveShadows == 0)
		return Lo;

	float shadow = 0.0;

	// first cascade
	vec4 vertexInLightSpace = directionalLight.CSM0 * vec4(FS_IN.worldVertexPosition, 1.0);
	vec3 projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	if (projCoords.x <= 0.9 && projCoords.y <= 0.9 && projCoords.x >= 0.1 && projCoords.y >= 0.1)
	{
		shadow = shadowCalculationCSM0(vertexInLightSpace, normal, lightDirection);
		return (Lo * (1.0 - shadow));
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
			return (Lo * (1.0 - shadow));
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
			return (Lo * (1.0 - shadow));
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
			return (Lo * (1.0 - shadow));
		}
	}

	return (Lo * (1.0 - shadow));
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