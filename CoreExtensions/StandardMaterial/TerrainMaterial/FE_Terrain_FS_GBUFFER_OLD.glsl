@TerrainLayersTextures@

#define FE_TERRAIN_MAX_LAYERS 8
uniform float usedLayersCount;

struct materialPropertiesData
{
    float normalMapIntensity;
	float AOIntensity;
	float AOMapIntensity;
	float roughtness;
	float roughtnessMapIntensity;
	float metalness;
	float metalnessMapIntensity;
	float displacementMapIntensity;
	float tiling;
};

layout(std430, binding = 0) readonly buffer materialProperties
{
	materialPropertiesData materials[];
};

struct layerMaterialInfo
{
	vec4 albedo;
    vec3 normal;
	float AO;
	float roughtness;
	float metalness;
	float displacement;
};

layerMaterialInfo layerMaterial[FE_TERRAIN_MAX_LAYERS];

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

@Texture@ heightMap;
uniform vec2 tileMult;
@Texture@ layerMap;
@Texture@ layerMap2;

void main(void)
{
	vec4 finalColor = vec4(0);
	vec3 finalNormal = vec3(0);
	vec4 finalMaterialProperties = vec4(0);

	gPosition = FS_IN.worldPosition;
	
	for (int i = 0; i < usedLayersCount; i++)
	{
		vec2 tiledUV = FS_IN.UV / materials[i].tiling;
	
		layerMaterial[i].albedo = texture(textures[i * 3], tiledUV);
		
		// ******************* normals *******************
		layerMaterial[i].normal = texture(textures[i * 3 + 1], tiledUV).xyz;
		layerMaterial[i].normal = normalize(layerMaterial[i].normal * 2.0 - 1.0);
		layerMaterial[i].normal = normalize(FS_IN.TBN * layerMaterial[i].normal);
		layerMaterial[i].normal = mix(FS_IN.vertexNormal, layerMaterial[i].normal, materials[i].normalMapIntensity);
		// ******************* normals END *******************
		
		// ******************* material properties *******************
		vec4 layerMaterialPropertiesTemp = texture(textures[i * 3 + 2], tiledUV);
		layerMaterial[i].AO = layerMaterialPropertiesTemp.r * materials[i].AOMapIntensity;
		layerMaterial[i].AO = mix(materials[i].AOIntensity, layerMaterial[i].AO, materials[i].AOMapIntensity);
		layerMaterial[i].roughtness = layerMaterialPropertiesTemp.g * materials[i].roughtnessMapIntensity;
		layerMaterial[i].metalness = layerMaterialPropertiesTemp.b * materials[i].metalnessMapIntensity;
		layerMaterial[i].displacement = layerMaterialPropertiesTemp.a * materials[i].displacementMapIntensity;
		// ******************* material properties END *******************
	}
	
	vec4 firstLayersMap = texture(layerMap, FS_IN.UV / vec2(tileMult));
	vec4 secondLayersMap = texture(layerMap2, FS_IN.UV / vec2(tileMult));
	
	for (int i = 0; i < int(clamp(usedLayersCount, 0, 4)); i++)
	{
		finalColor += layerMaterial[i].albedo * firstLayersMap[i];
		finalNormal += layerMaterial[i].normal * firstLayersMap[i];
		
		finalMaterialProperties.r += layerMaterial[i].AO * firstLayersMap[i];
		finalMaterialProperties.g += layerMaterial[i].roughtness * firstLayersMap[i];
		finalMaterialProperties.b += layerMaterial[i].metalness * firstLayersMap[i];
		finalMaterialProperties.a += layerMaterial[i].displacement * firstLayersMap[i];
	}
	
	for (int i = 4; i < usedLayersCount; i++)
	{
		finalColor += layerMaterial[i].albedo * secondLayersMap[i - 4];
		finalNormal += layerMaterial[i].normal * secondLayersMap[i - 4];
		
		finalMaterialProperties.r += layerMaterial[i].AO * secondLayersMap[i - 4];
		finalMaterialProperties.g += layerMaterial[i].roughtness * secondLayersMap[i - 4];
		finalMaterialProperties.b += layerMaterial[i].metalness * secondLayersMap[i - 4];
		finalMaterialProperties.a += layerMaterial[i].displacement * secondLayersMap[i - 4];
	}
	
	gAlbedo = finalColor;
	gNormal = normalize(finalNormal);
	gMaterialProperties = finalMaterialProperties;
	
	gShaderProperties.r = 1; // 1 - shaderID
	gShaderProperties.g = texture(projectedMap, FS_IN.UV / tileMult).r;
	gShaderProperties.b = texture(projectedMap, FS_IN.UV / tileMult).g;
	gShaderProperties.a = texture(projectedMap, FS_IN.UV / tileMult).b;

	outColor = finalColor;
}
