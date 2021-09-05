#define MAX_LIGHTS 10
in VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	flat float materialIndex;
} FS_IN;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 gPosition;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gMaterialProperties;
layout (location = 5) out vec4 gShaderProperties;

@MaterialTextures@
uniform float FENormalMapIntensity;
uniform float FEAOIntensity;
uniform float FEAOMapIntensity;
uniform float FERoughtness;
uniform float FERoughtnessMapIntensity;
uniform float FEMetalness;
uniform float FEMetalnessMapIntensity;

uniform int debugFlag;
uniform int compactMaterialPacking;
vec4 materialProperties;

vec4 getAlbedo()
{
	vec4 result = vec4(0);
	if (FS_IN.materialIndex == 0.0)
	{
		result = texture(textures[textureBindings[0]], FS_IN.UV);
	}
	else if (FS_IN.materialIndex == 1.0)
	{
		result = texture(textures[textureBindings[6]], FS_IN.UV);
	}

	return result;
}

vec3 getNormal()
{
	vec3 result = vec3(0);
	if (textureBindings[1] != -1)
	{
		if (FS_IN.materialIndex == 0.0)
		{
			result = texture(textures[textureBindings[1]], FS_IN.UV).rgb;
		}
		else if (FS_IN.materialIndex == 1.0)
		{
			result = texture(textures[textureBindings[7]], FS_IN.UV).rgb;
		}

		result = normalize(result * 2.0 - 1.0);
		result = normalize(FS_IN.TBN * result);
		result = mix(FS_IN.vertexNormal, result, FENormalMapIntensity);
	}
	else
	{
		result = normalize(FS_IN.vertexNormal);
	}

	return result;
}

float getAO()
{
	float result = 1;

	if (compactMaterialPacking == 1)
	{
		result = materialProperties[textureChannels[2]];
		result *= FEAOMapIntensity;
		result = mix(FEAOIntensity, result, FEAOMapIntensity);

		return result;
	}

	if (FS_IN.materialIndex == 0.0)
	{
		if (textureBindings[2] != -1)
			result = texture(textures[textureBindings[2]], FS_IN.UV)[textureChannels[2]];
	}
	else if (FS_IN.materialIndex == 1.0)
	{
		if (textureBindings[8] != -1)
			result = texture(textures[textureBindings[8]], FS_IN.UV)[textureChannels[8]];
	}
	
	result *= FEAOMapIntensity;
	result = mix(FEAOIntensity, result, FEAOMapIntensity);

	return result;
}

float getRoughtness()
{
	float result = FERoughtness;

	if (compactMaterialPacking == 1)
	{
		result = materialProperties[textureChannels[3]] * FERoughtnessMapIntensity;
		return result;
	}

	if (FS_IN.materialIndex == 0.0)
	{
		if (textureBindings[3] != -1)
			result = texture(textures[textureBindings[3]], FS_IN.UV)[textureChannels[3]] * FERoughtnessMapIntensity;
	}
	else if (FS_IN.materialIndex == 1.0)
	{
		if (textureBindings[9] != -1)
			result = texture(textures[textureBindings[9]], FS_IN.UV)[textureChannels[9]] * FERoughtnessMapIntensity;
	}

	return result;
}

float getMetalness()
{
	float result = FEMetalness;

	if (compactMaterialPacking == 1)
	{
		result = materialProperties[textureChannels[4]] * FEMetalnessMapIntensity;
		//result = 0.1;
		return result;
	}

	if (FS_IN.materialIndex == 0.0)
	{
		if (textureBindings[4] != -1)
			result = texture(textures[textureBindings[4]], FS_IN.UV)[textureChannels[4]] * FEMetalnessMapIntensity;
	}
	else if (FS_IN.materialIndex == 1.0)
	{
		if (textureBindings[10] != -1)
			result = texture(textures[textureBindings[10]], FS_IN.UV)[textureChannels[10]] * FEMetalnessMapIntensity;
	}

	return result;
}

float getDisplacement()
{
	float result = 0;

	if (compactMaterialPacking == 1)
	{
		result = materialProperties[textureChannels[5]] * FEMetalnessMapIntensity;
		return result;
	}

	if (FS_IN.materialIndex == 0.0)
	{
		if (textureBindings[5] != -1)
			result = texture(textures[textureBindings[5]], FS_IN.UV)[textureChannels[5]];
	}
	else if (FS_IN.materialIndex == 1.0)
	{
		if (textureBindings[11] != -1)
			result = texture(textures[textureBindings[11]], FS_IN.UV)[textureChannels[11]];
	}

	return result;
}

void main(void)
{
	if (compactMaterialPacking == 1)
	{
		if (FS_IN.materialIndex == 0.0)
		{
			materialProperties = texture(textures[textureBindings[2]], FS_IN.UV);
		}
		else if (FS_IN.materialIndex == 1.0)
		{
			materialProperties = texture(textures[textureBindings[8]], FS_IN.UV);
		}
	}

	vec4 textureColor = getAlbedo();
	if (textureColor.a < 0.2)
	{
		discard;
	}
	
	gPosition = FS_IN.worldPosition;
	gNormal = getNormal();
	gAlbedo = textureColor;
	gMaterialProperties.r = getAO();
	gMaterialProperties.g = getRoughtness();
	gMaterialProperties.b = getMetalness();
	gMaterialProperties.a = getDisplacement();
	gShaderProperties.r = 0; // 0 - shaderID
	
	outColor = textureColor;
}
