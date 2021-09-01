in vec2 textureCoords;

@ViewMatrix@
@ProjectionMatrix@

@MaterialTextures@

#define SSAO_SAMPLES_COUNT 8
vec3 SSAOOffsets[16] = vec3[](vec3(-0.613392, 0.617481, 0.791925), vec3(0.170019, -0.040254, -0.668203),
							  vec3(-0.299417, 0.791925, -0.271096), vec3(0.645680, 0.493210, 0.027070),
							  vec3(-0.651784, 0.717887, 0.421003), vec3(0.421003, 0.027070, -0.817194),
							  vec3(-0.817194, -0.271096, -0.705374), vec3(-0.705374, -0.668203, 0.651784),
							  vec3(-0.772454, -0.090976, 0.751784), vec3(0.504440, 0.372295, -0.368203),
							  vec3(0.155736, 0.065157, -0.190976), vec3(0.391522, 0.849605, -0.205374),
							  vec3(-0.620106, -0.328104, 0.6223403), vec3(0.789239, -0.419965, -0.070254),
							  vec3(-0.545396, 0.538133, 0.2945617), vec3(-0.178564, -0.596057, -0.493210));

vec3 getWorldPosition()
{
	return texture(textures[3], textureCoords.xy).rgb;
}

layout (location = 0) out vec4 outColor;

void main(void)
{
	float totalOcclusion = 0.0;
	float bias = 0.025;
	//float radius = 2.0;

	vec4 viewPosition = FEViewMatrix * vec4(getWorldPosition(), 1.0);
	
	vec4 offset = FEProjectionMatrix * viewPosition;
	offset.xyz /= offset.w;
	offset.xyz = offset.xyz * 0.5 + 0.5;
	float currentSampleZ = (FEViewMatrix * vec4(texture(textures[3], vec2(offset.x, offset.y)).rgb, 1.0)).z;
	
	float rotFactor = fract(sin(dot(viewPosition.xy * 19.19, vec2(49.5791, 97.413))) * 49831.189237);
	for (int i = 0; i < SSAO_SAMPLES_COUNT; i++)
	{
		vec4 sampleViewPosition = FEViewMatrix * vec4(getWorldPosition() + SSAOOffsets[i] * rotFactor, 1.0);
		//offset = FEProjectionMatrix * sampleViewPosition;
		offset = FEProjectionMatrix * vec4(viewPosition.xyz + SSAOOffsets[i] * rotFactor, viewPosition.w);
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;
	
		float surroundSampleZ = (FEViewMatrix * vec4(texture(textures[3], vec2(offset.x, offset.y)).rgb, 1.0)).z;
	
		//surroundSampleZ += bias;
		//float difference = currentSampleZ - surroundSampleZ;
		//totalOcclusion += (difference + abs(difference)) / 2.0;
	
		totalOcclusion += (currentSampleZ > surroundSampleZ + bias ? 1.0 : 0.0);
		//totalOcclusion += surroundSampleZ;
	
		//float rangeCheck = smoothstep(0.0, 1.0, radius / abs(currentSampleZ - surroundSampleZ));
		//totalOcclusion += (currentSampleZ > surroundSampleZ + bias ? 1.0 : 0.0) * rangeCheck;
	}

	outColor.r = totalOcclusion / SSAO_SAMPLES_COUNT;
}