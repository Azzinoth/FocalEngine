uniform float hightScale;
uniform float scaleFactor;
uniform vec2 tileMult;

@MaterialTextures@
uniform float FENormalMapPresent;
uniform float FENormalMapIntensity;
uniform float FEAOMapPresent;
uniform float FEAOIntensity;
uniform float FEAOMapIntensity;
uniform float FERoughnessMapPresent;
uniform float FERoughness;
uniform float FERoughnessMapIntensity;
uniform float FEMetalness;
uniform float FEMetalnessMapPresent;
uniform float FEMetalnessMapIntensity;

#define MAX_LIGHTS 10
in GS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
} FS_IN;

@ViewMatrix@
@ProjectionMatrix@

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 gPosition;
layout (location = 2) out float SSAOResult;

@CameraPosition@
uniform float FEGamma;
uniform int debugFlag;
uniform float shadowBlurFactor;

uniform float fogDensity;
uniform float fogGradient;

struct FELight
{
	vec4 typeAndAngles;
	vec4 position;
	vec4 color;
	vec4 direction;
	mat4 lightSpace;
};

// is object receiving shadows.
@RECEVESHADOWS@
// adds cascade shadow maps, 4 cascades.
@CSM@

@Texture@ heightMap;
@Texture@ projectedMap;
@Texture@ lastFrameSceneDepth;
@Texture@ lastFramePositionsGBuffer;
@Texture@ lastFrameSSAO;

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
	vec4 CSMSizes;
	int activeCascades;
	float biasFixed;
	float biasVariableIntensity;
	float intensity;
};

#define CSM_MAX_CASCADE 3

layout (set = 0, binding = 1, std140) uniform directionalLightInfo
{
	FEDirectionalLight directionalLight;
};

vec4 getAlbedo()
{
	vec4 result = vec4(0);
	result = texture(textures[textureBindings[0]], FS_IN.UV);
	return result;
}

vec3 getNormal()
{
	vec3 result = vec3(0);
	if (textureBindings[1] != -1)
	{
		result = texture(textures[textureBindings[1]], FS_IN.UV).rgb;
		result = normalize(result * 2.0 - 1.0);
		result = normalize(FS_IN.TBN * result);
		result = mix(FS_IN.vertexNormal, result, FENormalMapIntensity);
	}
	else
	{
		//result = FS_IN.vertexNormal;
		result = normalize(FS_IN.TBN * FS_IN.vertexNormal);
	}

	return result;
}

float getAO()
{
	float result = 1;

	if (textureBindings[2] != -1)
		result = texture(textures[textureBindings[2]], FS_IN.UV)[textureChannels[2]];

	vec4 fragmentInScreenSpace = FEProjectionMatrix * FS_IN.viewPosition;
	fragmentInScreenSpace.xyz /= fragmentInScreenSpace.w;
	fragmentInScreenSpace.xyz  = fragmentInScreenSpace.xyz * 0.5 + 0.5;
	float SSAOIntensity = texture(lastFrameSSAO, vec2(fragmentInScreenSpace.x, fragmentInScreenSpace.y)).r;

	//return result;

	//if (result != 0)
		return result * SSAOIntensity;

	//return SSAOIntensity;
}

float getRoughness()
{
	float result = FERoughness;

	if (textureBindings[3] != -1)
		result = texture(textures[textureBindings[3]], FS_IN.UV)[textureChannels[3]] * FERoughnessMapIntensity;

	return result;
}

float getMetalness()
{
	float result = FEMetalness;

	if (textureBindings[4] != -1)
		result = texture(textures[textureBindings[4]], FS_IN.UV)[textureChannels[4]] * FEMetalnessMapIntensity;

	return result;
}

float getDisplacement()
{
	float result = 0;

	if (textureBindings[5] != -1)
		result = texture(textures[textureBindings[5]], FS_IN.UV)[textureChannels[5]];
	
	return result;
}

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

//#define NUM_BLUR_TAPS 16
//
//vec2 filterTaps[NUM_BLUR_TAPS] = vec2[](
//	vec2(-0.94201624, -0.39906216),
//	vec2(0.94558609, -0.76890725),
//	vec2(-0.094184101, -0.92938870),
//	vec2(0.34495938, 0.29387760),
//	vec2(-0.91588581, 0.45771432),
//	vec2(-0.81544232, -0.87912464),
//	vec2(-0.38277543, 0.27676845),
//	vec2(0.97484398, 0.75648379),
//	vec2(0.44323325, -0.97511554),
//	vec2(0.53742981, -0.47373420),
//	vec2(-0.26496911, -0.41893023),
//	vec2(0.79197514, 0.19090188),
//	vec2(-0.24188840, 0.99706507),
//	vec2(-0.81409955, 0.91437590),
//	vec2(0.19984126, 0.78641367),
//	vec2(0.14383161, -0.14100790));

//#define NUM_BLUR_TAPS 12
//
//vec2 filterTaps[NUM_BLUR_TAPS] = vec2[] ( vec2(-0.326212, -0.405805), vec2(-0.840144, -0.07358),
//										  vec2(-0.695914,  0.457137), vec2(-0.203345,  0.620716),
//										  vec2( 0.96234,  -0.194983), vec2( 0.473434, -0.480026),
//										  vec2( 0.519456,  0.767022), vec2( 0.185461, -0.893124),
//										  vec2( 0.507431,  0.064425), vec2( 0.89642,   0.412458),
//										  vec2(-0.32194,  -0.932615), vec2(-0.791559, -0.597705));

#define NUM_BLUR_TAPS 64
vec2 filterTaps[NUM_BLUR_TAPS] = vec2[](
	vec2(-0.613392, 0.617481), vec2(0.170019, -0.040254),
	vec2(-0.299417, 0.791925), vec2(0.645680, 0.493210),
	vec2(-0.651784, 0.717887), vec2(0.421003, 0.027070),
	vec2(-0.817194, -0.271096), vec2(-0.705374, -0.668203),
	vec2(0.977050, -0.108615), vec2(0.063326, 0.142369),
	vec2(0.203528, 0.214331), vec2(-0.667531, 0.326090),
	vec2(-0.098422, -0.295755), vec2(-0.885922, 0.215369),
	vec2(0.566637, 0.605213), vec2(0.039766, -0.396100),
	vec2(0.751946, 0.453352), vec2(0.078707, -0.715323),
	vec2(-0.075838, -0.529344), vec2(0.724479, -0.580798),
	vec2(0.222999, -0.215125), vec2(-0.467574, -0.405438),
	vec2(-0.248268, -0.814753), vec2(0.354411, -0.887570),
	vec2(0.175817, 0.382366), vec2(0.487472, -0.063082),
	vec2(-0.084078, 0.898312), vec2(0.488876, -0.783441),
	vec2(0.470016, 0.217933), vec2(-0.696890, -0.549791),
	vec2(-0.149693, 0.605762), vec2(0.034211, 0.979980),
	vec2(0.503098, -0.308878), vec2(-0.016205, -0.872921),
	vec2(0.385784, -0.393902), vec2(-0.146886, -0.859249),
	vec2(0.643361, 0.164098), vec2(0.634388, -0.049471),
	vec2(-0.688894, 0.007843), vec2(0.464034, -0.188818),
	vec2(-0.440840, 0.137486), vec2(0.364483, 0.511704),
	vec2(0.034028, 0.325968), vec2(0.099094, -0.308023),
	vec2(0.693960, -0.366253), vec2(0.678884, -0.204688),
	vec2(0.001801, 0.780328), vec2(0.145177, -0.898984),
	vec2(0.062655, -0.611866), vec2(0.315226, -0.604297),
	vec2(-0.780145, 0.486251), vec2(-0.371868, 0.882138),
	vec2(0.200476, 0.494430), vec2(-0.494552, -0.711051),
	vec2(0.612476, 0.705252), vec2(-0.578845, -0.768792),
	vec2(-0.772454, -0.090976), vec2(0.504440, 0.372295),
	vec2(0.155736, 0.065157), vec2(0.391522, 0.849605),
	vec2(-0.620106, -0.328104), vec2(0.789239, -0.419965),
	vec2(-0.545396, 0.538133), vec2(-0.178564, -0.596057));

#define SSAO_SAMPLES_COUNT 8
vec3 SSAOOffsets[16] = vec3[](vec3(-0.613392, 0.617481, 0.791925), vec3(0.170019, -0.040254, -0.668203),
												vec3(-0.299417, 0.791925, -0.271096), vec3(0.645680, 0.493210, 0.027070),
												vec3(-0.651784, 0.717887, 0.421003), vec3(0.421003, 0.027070, -0.817194),
												vec3(-0.817194, -0.271096, -0.705374), vec3(-0.705374, -0.668203, 0.651784),
												vec3(-0.772454, -0.090976, 0.751784), vec3(0.504440, 0.372295, -0.368203),
												vec3(0.155736, 0.065157, -0.190976), vec3(0.391522, 0.849605, -0.205374),
												vec3(-0.620106, -0.328104, 0.6223403), vec3(0.789239, -0.419965, -0.070254),
												vec3(-0.545396, 0.538133, 0.2945617), vec3(-0.178564, -0.596057, -0.493210));

void main(void)
{
	//vec4 testView = inverse(FEViewMatrix) * FS_IN.viewPosition;
	//gPosition = FS_IN.viewPosition.z;
	//gPosition = (FEViewMatrix * vec4(FS_IN.worldPosition, 1.0)).z;
	gPosition = FS_IN.worldPosition;

	float totalOcclusion = 0.0;
	float bias = 0.025;
	//float radius = 2.0;

	vec4 viewPosition = FEViewMatrix * vec4(FS_IN.worldPosition, 1.0);
	
	vec4 offset = FEProjectionMatrix * viewPosition;
	offset.xyz /= offset.w;
	offset.xyz = offset.xyz * 0.5 + 0.5;
	float currentSampleZ = (FEViewMatrix * vec4(texture(lastFramePositionsGBuffer, vec2(offset.x, offset.y)).rgb, 1.0)).z;
	
	float rotFactor = fract(sin(dot(viewPosition.xy * 19.19, vec2(49.5791, 97.413))) * 49831.189237);
	for (int i = 0; i < SSAO_SAMPLES_COUNT; i++)
	{
		vec4 sampleViewPosition = FEViewMatrix * vec4(FS_IN.worldPosition + SSAOOffsets[i] * rotFactor, 1.0);
		//offset = FEProjectionMatrix * sampleViewPosition;
		offset = FEProjectionMatrix * vec4(viewPosition.xyz + SSAOOffsets[i] * rotFactor, viewPosition.w);
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;
	
		float surroundSampleZ = (FEViewMatrix * vec4(texture(lastFramePositionsGBuffer, vec2(offset.x, offset.y)).rgb, 1.0)).z;
	
		//surroundSampleZ += bias;
		//float difference = currentSampleZ - surroundSampleZ;
		//totalOcclusion += (difference + abs(difference)) / 2.0;
	
		totalOcclusion += (currentSampleZ > surroundSampleZ + bias ? 1.0 : 0.0);
		//totalOcclusion += surroundSampleZ;
	
		//float rangeCheck = smoothstep(0.0, 1.0, radius / abs(currentSampleZ - surroundSampleZ));
		//totalOcclusion += (currentSampleZ > surroundSampleZ + bias ? 1.0 : 0.0) * rangeCheck;
	}
	SSAOResult = totalOcclusion / SSAO_SAMPLES_COUNT;




	// checking viewDirection
	if (debugFlag == 1)
	{
		outColor = vec4(vec3(dot(getNormal(), normalize(FECameraPosition - FS_IN.worldPosition))), 1.0);
		return;
	}
	// checking normals
	if (debugFlag == 2)
	{
		outColor = vec4(getNormal(), 1.0);
		return;
	}
	// checking UV
	if (debugFlag == 3)
	{
		outColor = vec4(FS_IN.UV.x, FS_IN.UV.y, 0.0, 1.0);
		return;
	}
	// debug csm
	else if (debugFlag == 4)
	{
		float distanceToCam = length(FECameraPosition - FS_IN.worldPosition);

		// CSM0
		if (distanceToCam <= directionalLight.CSMSizes[0])
		{
			outColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
		}
		
		// CSM1
		if (directionalLight.activeCascades > 1)
		{
			if (distanceToCam <= directionalLight.CSMSizes[1])
			{
				outColor = vec4(0.0, 0.0, 1.0, 1.0);
				return;
			}
		}

		// CSM2
		if (directionalLight.activeCascades > 2)
		{
			if (distanceToCam <= directionalLight.CSMSizes[2])
			{
				outColor = vec4(1.0, 1.0, 0.0, 1.0);
				return;
			}
		}

		// CSM3
		if (directionalLight.activeCascades > 3)
		{
			if (distanceToCam <= directionalLight.CSMSizes[3])
			{
				outColor = vec4(1.0, 0.0, 0.0, 1.0);
				return;
			}
		}
	}

	vec4 textureColor = getAlbedo();
	if (textureColor.a < 0.2)
	{
		discard;
	}
		
	vec3 baseColor = pow(textureColor.rgb, vec3(FEGamma));
	vec3 viewDirection = normalize(FECameraPosition - FS_IN.worldPosition);
	vec3 ambientColor = baseColor * 0.09f + vec3(0.55f, 0.73f, 0.87f) * 0.009f;

	vec3 normal = getNormal();
	
	vec3 ao_base = ambientColor * FEAOIntensity * (directionalLight.intensity / 16.0);
	float textureAO = getAO();
	if (textureAO != 0)
	{
		outColor = vec4(mix(ao_base, ambientColor * textureAO * (directionalLight.intensity / 8.0), FEAOMapIntensity), 1.0f);
	}
	else
	{
		outColor = vec4(ao_base, 1.0f);
	}

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (FElight[i].color.x == 0 && FElight[i].color.y == 0 && FElight[i].color.z == 0)
			continue;

		if (FElight[i].typeAndAngles.x == 2)
		{
			outColor += vec4(pointLightColor(FElight[i], normal, FS_IN.worldPosition, viewDirection, baseColor), 1.0f);
		}
		else if (FElight[i].typeAndAngles.x == 3)
		{
			outColor += vec4(spotLightColor(FElight[i], normal, FS_IN.worldPosition, viewDirection, baseColor), 1.0f);
		}
	}

	outColor += vec4(directionalLightColor(normal, FS_IN.worldPosition, viewDirection, baseColor), 1.0f);
	outColor += vec4(texture(projectedMap, FS_IN.UV / tileMult));

	// test fog
	if (fogDensity > 0.0f && fogGradient > 0.0f)
	{
		float distanceToCam = length(FECameraPosition - FS_IN.worldPosition);

		float visibility = exp(-pow((distanceToCam * fogDensity), fogGradient));
		visibility = clamp(visibility, 0.0, 1.0);

		outColor = mix(vec4(0.55f, 0.73f, 0.87f, 1.0), outColor, visibility);
	}
}

// Produces cheap but low-quality white noise, nothing special
// Taken from Godot engine
float quick_hash(vec2 pos)
{
	return fract(sin(dot(pos * 19.19, vec2(49.5791, 97.413))) * 49831.189237);
}

float getBias(vec3 normal, vec3 lightDir)
{
	if (directionalLight.biasFixed != -1)
	{
		return directionalLight.biasFixed;
	}
	else
	{
		float minBias = 0.001 * directionalLight.biasVariableIntensity;
		float baseBias = 0.002 * directionalLight.biasVariableIntensity;
		return max(baseBias * (1.0 - dot(normal, lightDir)), minBias);
	}
}

float shadowCalculationCSM0(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	float shadow = 0.0;

	//vec4 vertex = vec4(FS_IN.worldPosition, 1.0);
	//vertex.xyz += lightDir * 0.01;
	//vec3 normal_bias = normalize(normal) * (1.0 - max(0.0, dot(lightDir, -normalize(normal)))) * 0.8;
	//normal_bias -= lightDir * dot(lightDir, normal_bias);
	//vertex.xyz += normal_bias;

	//fragPosLightSpace = directionalLight.CSM0 * vertex;
	
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(CSM0, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = getBias(normal, lightDir);
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
		float pcfDepth = texture(CSM0, projCoords.xy + disk_rotation * filterTaps[i] * shadowBlurFactor * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		//shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
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
	float bias = getBias(normal, lightDir);
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
		float pcfDepth = texture(CSM1, projCoords.xy + disk_rotation * filterTaps[i] * shadowBlurFactor * texelSize).r;
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
	float bias = getBias(normal, lightDir);
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
		float pcfDepth = texture(CSM2, projCoords.xy + disk_rotation * filterTaps[i] * shadowBlurFactor * texelSize).r;
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
	float bias = getBias(normal, lightDir);
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
		float pcfDepth = texture(CSM3, projCoords.xy + disk_rotation * filterTaps[i] * shadowBlurFactor * texelSize).r;
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	}
	shadow = shadow/NUM_BLUR_TAPS;

	return shadow;
}

vec3 directionalLightColor(vec3 normal, vec3 fragPosition, vec3 viewDir, vec3 baseColor)
{
	vec3 lightDirection = normalize(-directionalLight.direction.xyz);
	// only sun light
	if (lightDirection.y < 0)
		return vec3(0.0);

	vec3 albedo = baseColor;
	float metallic = getMetalness();
	float roughness = getRoughness();

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
    float denominator = 4 * max(dot(N, V), 0.1) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
    // brutal hack
    //specular = specular * 0.08 * (20.0 * (metallic + 0.01));

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

	if (!FEReceiveShadows)
		return Lo;

	float shadow = 0.0;
	float distanceToCam = length(FECameraPosition - FS_IN.worldPosition);

	// first cascade
	vec4 vertexInLightSpace = directionalLight.CSM0 * vec4(FS_IN.worldPosition, 1.0);
	vec3 projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	if (distanceToCam <= directionalLight.CSMSizes[0])
	{
		shadow = shadowCalculationCSM0(vertexInLightSpace, normal, lightDirection);
		return (Lo * (1.0 - shadow));
	}
	
	// second cascade
	if (directionalLight.activeCascades > 1)
	{
		vertexInLightSpace = directionalLight.CSM1 * vec4(FS_IN.worldPosition, 1.0);
		projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (distanceToCam <= directionalLight.CSMSizes[1])
		{
			shadow = shadowCalculationCSM1(vertexInLightSpace, normal, lightDirection);
			return (Lo * (1.0 - shadow));
		}
	}

	// third cascade
	if (directionalLight.activeCascades > 2)
	{
		vertexInLightSpace = directionalLight.CSM2 * vec4(FS_IN.worldPosition, 1.0);
		projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (distanceToCam <= directionalLight.CSMSizes[2])
		{
			shadow = shadowCalculationCSM2(vertexInLightSpace, normal, lightDirection);
			return (Lo * (1.0 - shadow));
		}
	}

	// fourth(last) cascade
	if (directionalLight.activeCascades > 3)
	{
		vertexInLightSpace = directionalLight.CSM3 * vec4(FS_IN.worldPosition, 1.0);
		projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (distanceToCam <= directionalLight.CSMSizes[3])
		{
			shadow = shadowCalculationCSM3(vertexInLightSpace, normal, lightDirection);
			// fading of last cascade
			shadow *= clamp((directionalLight.CSMSizes[3] - distanceToCam) / (directionalLight.CSMSizes[3] * 0.1), 0.0, 1.0);
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