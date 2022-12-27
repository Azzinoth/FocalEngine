in vec2 textureCoords;

@ViewMatrix@
@ProjectionMatrix@

@MaterialTextures@

vec3 SSAOOffsets[64] = vec3[](vec3(0.049771, -0.044709, 0.049963), vec3(0.014575, 0.016531, 0.002239), vec3(-0.040648, -0.019375, 0.031934), 
							  vec3(0.013778, -0.091582, 0.040924), vec3(0.055989, 0.059792, 0.057659), vec3(0.092266, 0.044279, 0.015451), 
							  vec3(-0.002039, -0.054402, 0.066735), vec3(-0.000331, -0.000187, 0.000369), vec3(0.050045, -0.046650, 0.025385), 
							  vec3(0.038128, 0.031402, 0.032868), vec3(-0.031883, 0.020459, 0.022515), vec3(0.055702, -0.036974, 0.054492), 
							  vec3(0.057372, -0.022540, 0.075542), vec3(-0.016090, -0.003768, 0.055473), vec3(-0.025033, -0.024829, 0.024951), 
							  vec3(-0.033688, 0.021391, 0.025402), vec3(-0.017530, 0.014386, 0.005348), vec3(0.073359, 0.112052, 0.011015), 
							  vec3(-0.044056, -0.090284, 0.083683), vec3(-0.083277, -0.001683, 0.084987), vec3(-0.010406, -0.032867, 0.019273), 
							  vec3(0.003211, -0.004882, 0.004164), vec3(-0.007383, -0.065835, 0.067398), vec3(0.094141, -0.007998, 0.143350), 
							  vec3(0.076833, 0.126968, 0.106999), vec3(0.000393, 0.000450, 0.000302), vec3(-0.104793, 0.065445, 0.101737), 
							  vec3(-0.004452, -0.119638, 0.161901), vec3(-0.074553, 0.034449, 0.224138), vec3(-0.002758, 0.003078, 0.002923), 
							  vec3(-0.108512, 0.142337, 0.166435), vec3(0.046882, 0.103636, 0.059576), vec3(0.134569, -0.022512, 0.130514), 
							  vec3(-0.164490, -0.155644, 0.124540), vec3(-0.187666, -0.208834, 0.057770), vec3(-0.043722, 0.086925, 0.074797), 
							  vec3(-0.002564, -0.002001, 0.004070), vec3(-0.096696, -0.182259, 0.299487), vec3(-0.225767, 0.316061, 0.089156), 
							  vec3(-0.027505, 0.287187, 0.317177), vec3(0.207216, -0.270839, 0.110132), vec3(0.054902, 0.104345, 0.323106), 
							  vec3(-0.130860, 0.119294, 0.280219), vec3(0.154035, -0.065371, 0.229842), vec3(0.052938, -0.227866, 0.148478), 
							  vec3(-0.187305, -0.040225, 0.015926), vec3(0.141843, 0.047163, 0.134847), vec3(-0.044268, 0.055616, 0.055859), 
							  vec3(-0.023583, -0.080970, 0.219130), vec3(-0.142147, 0.198069, 0.005194), vec3(0.158646, 0.230457, 0.043715), 
							  vec3(0.030040, 0.381832, 0.163825), vec3(0.083006, -0.309661, 0.067413), vec3(0.226953, -0.235350, 0.193673), 
							  vec3(0.381287, 0.332041, 0.529492), vec3(-0.556272, 0.294715, 0.301101), vec3(0.424490, 0.005647, 0.117578), 
							  vec3(0.366500, 0.003588, 0.085702), vec3(0.329018, 0.030898, 0.178504), vec3(-0.082938, 0.512848, 0.056555), 
							  vec3(0.867363, -0.002734, 0.100138), vec3(0.455745, -0.772006, 0.003841), vec3(0.417291, -0.154846, 0.462514), vec3(-0.442722, -0.679282, 0.186503));

vec3 getWorldPosition()
{
	return texture(textures[3], textureCoords.xy).rgb;
}

layout (location = 0) out vec4 outColor;

uniform int SampleCount;

uniform float SmallDetails;
uniform float SmallDetailsWeight;

uniform float Bias;
uniform float Radius;
uniform float RadiusSmallDetails;

vec2 WorldPositionToScreenUV(vec3 Position)
{
	vec4 ScreenUV = FEProjectionMatrix * FEViewMatrix * vec4(Position, 1.0);
	ScreenUV.xyz /= ScreenUV.w;
	ScreenUV.xyz = ScreenUV.xyz * 0.5 + 0.5;
	
	return ScreenUV.xy;
}

void main(void)
{
	float totalOcclusion = 0.0;
	vec3 CurrentNormal = texture(textures[1], textureCoords.xy).rgb;
	
	vec2 ScreenUV = WorldPositionToScreenUV(getWorldPosition());
	float PixelZ = (FEViewMatrix * vec4(texture(textures[3], ScreenUV).rgb, 1.0)).z;
	
	float rotFactor = fract(sin(dot(ScreenUV * 19.19, vec2(49.5791, 97.413))) * 49831.189237);
	for (int i = 0; i < SampleCount; i++)
	{
		vec3 CurrentSampleVectorOffset = SSAOOffsets[i] * Radius * rotFactor;
		if (dot(CurrentNormal, CurrentSampleVectorOffset) < 0.0)
			CurrentSampleVectorOffset = -CurrentSampleVectorOffset;
			
		vec2 ScreenUV = WorldPositionToScreenUV(getWorldPosition() + CurrentSampleVectorOffset);
		float CurrentSampleZ = (FEViewMatrix * vec4(texture(textures[3], ScreenUV).rgb, 1.0)).z;
	
		if (PixelZ < CurrentSampleZ - Bias)
		{
			totalOcclusion += 1.0;
			if (Radius < abs(PixelZ - (CurrentSampleZ - Bias)))
				totalOcclusion -= 1.0;
		}
	}

	outColor.r = 1.0f - totalOcclusion / SampleCount;
	
	if (SmallDetails > 0.0f)
	{
		totalOcclusion = 0.0;
	
		for (int i = 0; i < SampleCount; i++)
		{
			vec3 CurrentSampleVectorOffset = SSAOOffsets[i] * RadiusSmallDetails * rotFactor;
			if (dot(CurrentNormal, CurrentSampleVectorOffset) < 0.0)
				CurrentSampleVectorOffset = -CurrentSampleVectorOffset;
			
			ScreenUV = WorldPositionToScreenUV(getWorldPosition() + CurrentSampleVectorOffset);
			float CurrentSampleZ = (FEViewMatrix * vec4(texture(textures[3], ScreenUV).rgb, 1.0)).z;
	
			if (PixelZ < CurrentSampleZ - Bias)
			{
				totalOcclusion += 1.0;
				if (RadiusSmallDetails < abs(PixelZ - (CurrentSampleZ - Bias)))
					totalOcclusion -= 1.0;
			}
		}

		outColor.r -= (1.0f - totalOcclusion / SampleCount) * SmallDetailsWeight;
	}

	outColor.r = clamp(outColor.r, 0.0f, 1.0f);
}