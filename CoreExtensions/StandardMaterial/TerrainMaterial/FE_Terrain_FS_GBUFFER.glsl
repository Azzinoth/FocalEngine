uniform vec2 tileMult;

@MaterialTextures@
uniform float FENormalMapPresent;
uniform float FENormalMapIntensity;
uniform float FEAOMapPresent;
uniform float FEAOIntensity;
uniform float FEAOMapIntensity;
uniform float FERoughtnessMapPresent;
uniform float FERoughtness;
uniform float FERoughtnessMapIntensity;
uniform float FEMetalness;
uniform float FEMetalnessMapPresent;
uniform float FEMetalnessMapIntensity;

in GS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
} FS_IN;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 gPosition;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gMaterialProperties;
layout (location = 5) out vec4 gShaderProperties;

@Texture@ projectedMap;

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
		result = normalize(FS_IN.TBN * FS_IN.vertexNormal);
	}

	return result;
}

float getAO()
{
	float result = 1;

	if (textureBindings[2] != -1)
		result = texture(textures[textureBindings[2]], FS_IN.UV)[textureChannels[2]] * FEAOMapIntensity;

	result *= FEAOMapIntensity;
	result = mix(FEAOIntensity, result, FEAOMapIntensity);
	
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

void main(void)
{
	vec4 textureColor = getAlbedo();
	//if (textureColor.a < 0.2)
	//{
	//	discard;
	//}
	
	gPosition = FS_IN.worldPosition;
	gNormal = getNormal();
	gAlbedo = textureColor;
	gMaterialProperties.r = getAO();
	gMaterialProperties.g = getRoughtness();
	gMaterialProperties.b = getMetalness();
	gMaterialProperties.a = getDisplacement();
	gShaderProperties.r = 1; // 1 - shaderID
	gShaderProperties.g = texture(projectedMap, FS_IN.UV / tileMult).r;
	gShaderProperties.b = texture(projectedMap, FS_IN.UV / tileMult).g;
	gShaderProperties.a = texture(projectedMap, FS_IN.UV / tileMult).b;
	
	outColor = textureColor;
}
