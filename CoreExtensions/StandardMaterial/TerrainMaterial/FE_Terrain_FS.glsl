#version 450 core

uniform float hightScale;
uniform float scaleFactor;
uniform vec2 tileMult;

@MaterialTextures@

//Texture@ albedoMap;
//Texture@ normalMap;
uniform float FENormalMapPresent;
uniform float FENormalMapIntensity;
//Texture@ AOMap;
uniform float FEAOMapPresent;
uniform float FEAOIntensity;
uniform float FEAOMapIntensity;
//Texture@ roughtnessMap;
uniform float FERoughtnessMapPresent;
uniform float FERoughtness;
uniform float FERoughtnessMapIntensity;
//Texture@ metalnessMap;
uniform float FEMetalness;
uniform float FEMetalnessMapPresent;
uniform float FEMetalnessMapIntensity;

#define MAX_LIGHTS 10
in GS_OUT
{
	vec2 UV;
	vec3 fragPosition;
	vec3 worldVertexPosition;
	mat3 TBN;
	vec3 vertexNormal;
} FS_IN;

@CameraPosition@
uniform float FEGamma;
uniform int debugFlag;
uniform float shadowBlurFactor;

uniform float fogDensity;
uniform float fogGradient;

struct FELight
{
	vec3 typeAndAngles;
	vec3 position;
	vec3 color;
	vec3 direction;
	mat4 lightSpace;
};

// is object receiving shadows.
@RECEVESHADOWS@
// adds cascade shadow maps, 4 cascades.
@CSM@

@Texture@ heightMap;
@Texture@ projectedMap;

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
		result = FS_IN.vertexNormal;
	}

	return result;
}

float getAO()
{
	float result = 0;

	if (textureBindings[2] != -1)
		result = texture(textures[textureBindings[2]], FS_IN.UV)[textureChannels[2]];
	
	return result;
}

float getRoughtness()
{
	float result = FERoughtness;

	if (textureBindings[3] != -1)
		result = texture(textures[textureBindings[3]], FS_IN.UV)[textureChannels[3]] * FERoughtnessMapIntensity;

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

#define NUM_BLUR_TAPS 12

vec2 filterTaps[NUM_BLUR_TAPS] = vec2[] ( vec2(-0.326212, -0.405805), vec2(-0.840144, -0.07358),
										  vec2(-0.695914,  0.457137), vec2(-0.203345,  0.620716),
										  vec2( 0.96234,  -0.194983), vec2( 0.473434, -0.480026),
										  vec2( 0.519456,  0.767022), vec2( 0.185461, -0.893124),
										  vec2( 0.507431,  0.064425), vec2( 0.89642,   0.412458),
										  vec2(-0.32194,  -0.932615), vec2(-0.791559, -0.597705));

void main(void)
{
	// checking viewDirection
	if (debugFlag == 1)
	{
		gl_FragColor = vec4(vec3(dot(getNormal(), normalize(FECameraPosition - FS_IN.worldVertexPosition))), 1.0);
		return;
	}
	// checking normals
	if (debugFlag == 2)
	{
		gl_FragColor = vec4(getNormal(), 1.0);
		return;
	}
	// checking UV
	if (debugFlag == 3)
	{
		gl_FragColor = vec4(FS_IN.UV.x, FS_IN.UV.y, 0.0, 1.0);
		return;
	}
	// debug csm
	else if (debugFlag == 4)
	{
		float distanceToCam = length(FECameraPosition - FS_IN.fragPosition);

		// CSM0
		if (distanceToCam <= directionalLight.CSMSizes[0])
		{
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
		}
		
		// CSM1
		if (directionalLight.activeCascades > 1)
		{
			if (distanceToCam <= directionalLight.CSMSizes[1])
			{
				gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
				return;
			}
		}

		// CSM2
		if (directionalLight.activeCascades > 2)
		{
			if (distanceToCam <= directionalLight.CSMSizes[2])
			{
				gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
				return;
			}
		}

		// CSM3
		if (directionalLight.activeCascades > 3)
		{
			if (distanceToCam <= directionalLight.CSMSizes[3])
			{
				gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
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
	vec3 viewDirection = normalize(FECameraPosition - FS_IN.fragPosition);
	vec3 ambientColor = baseColor * 0.09f + vec3(0.55f, 0.73f, 0.87f) * 0.009f;

	vec3 normal = getNormal();
	
	vec3 ao_base = ambientColor * FEAOIntensity * (directionalLight.intensity / 16.0);
	float textureAO = getAO();
	if (textureAO != 0)
	{
		gl_FragColor = vec4(mix(ao_base, ambientColor * textureAO * (directionalLight.intensity / 8.0), FEAOMapIntensity), 1.0f);
	}
	else
	{
		gl_FragColor = vec4(ao_base, 1.0f);
	}

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
	gl_FragColor += vec4(texture(projectedMap, FS_IN.UV / tileMult));

	// test fog
	if (fogDensity > 0.0f && fogGradient > 0.0f)
	{
		float distanceToCam = length(FECameraPosition - FS_IN.fragPosition);

		float visibility = exp(-pow((distanceToCam * fogDensity), fogGradient));
		visibility = clamp(visibility, 0.0, 1.0);

		gl_FragColor = mix(vec4(0.55f, 0.73f, 0.87f, 1.0), gl_FragColor, visibility);
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

	//vec4 vertex = vec4(FS_IN.worldVertexPosition, 1.0);
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

	vec3 albedo = baseColor;
	float metallic = getMetalness();
	float roughness = getRoughtness();

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

	if (FEReceiveShadows == 0)
		return Lo;

	float shadow = 0.0;
	float distanceToCam = length(FECameraPosition - FS_IN.fragPosition);

	// first cascade
	vec4 vertexInLightSpace = directionalLight.CSM0 * vec4(FS_IN.worldVertexPosition, 1.0);
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
		vertexInLightSpace = directionalLight.CSM1 * vec4(FS_IN.worldVertexPosition, 1.0);
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
		vertexInLightSpace = directionalLight.CSM2 * vec4(FS_IN.worldVertexPosition, 1.0);
		projCoords = vertexInLightSpace.xyz / vertexInLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		if (distanceToCam <= directionalLight.CSMSizes[2])
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
		if (distanceToCam <= directionalLight.CSMSizes[3])
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