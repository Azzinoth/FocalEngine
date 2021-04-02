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
	if (textureColor.a < 0.2)
	{
		discard;
	}
		
	vec3 baseColor = pow(textureColor.rgb, vec3(FEGamma));
	vec3 viewDirection = normalize(FECameraPosition - FS_IN.fragPosition);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 0.3f;

    vec3 normal = texture(normalsTexture, FS_IN.UV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(FS_IN.TBN * normal);

	gl_FragColor = vec4(baseColor * ambientColor, 0.0f);
	
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (FElight[i].color.x == 0 && FElight[i].color.y == 0 && FElight[i].color.z == 0)
			continue;

		if (FElight[i].typeAndAngles.x == 10)
		{
			gl_FragColor += vec4(pointLightColor(FElight[i], normal, FS_IN.fragPosition, viewDirection, baseColor), 1.0f);
		}
		else if (FElight[i].typeAndAngles.x == 11)
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