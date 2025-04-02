in vec2 textureCoords;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 gPosition;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gMaterialProperties;
layout (location = 5) out vec4 gShaderProperties;
layout (location = 6) out vec2 gMotionVectors;

@Texture@ FirstColorTexture;
@Texture@ FirstPositionTexture;
@Texture@ FirstNormalTexture;
@Texture@ FirstAlbedoTexture;
@Texture@ FirstMaterialPropertiesTexture;
@Texture@ FirstShaderPropertiesTexture;
@Texture@ FirstMotionVectorsTexture;
@Texture@ FirstDepthTexture;

@Texture@ SecondColorTexture;
@Texture@ SecondPositionTexture;
@Texture@ SecondNormalTexture;
@Texture@ SecondAlbedoTexture;
@Texture@ SecondMaterialPropertiesTexture;
@Texture@ SecondShaderPropertiesTexture;
@Texture@ SecondMotionVectorsTexture;
@Texture@ SecondDepthTexture;

uniform float FirstNearPlane;
uniform float FirstFarPlane;
uniform float SecondNearPlane;
uniform float SecondFarPlane;

float LinearizeDepth(float NonLinearDepth, float NearPlane, float FarPlane)
{
	float NDCNormalizedDepth = 2.0 * NonLinearDepth - 1.0;
	return 2.0 * NearPlane * FarPlane / (FarPlane + NearPlane - NDCNormalizedDepth * (FarPlane - NearPlane));
}

void main()
{
	float FirstNonLinearDepth = texture(FirstDepthTexture, textureCoords).r;
	float FirstLinearDepth = LinearizeDepth(FirstNonLinearDepth, FirstNearPlane, FirstFarPlane);
	
	float SecondNonLinearDepth = texture(SecondDepthTexture, textureCoords).r;
	float SecondLinearDepth = LinearizeDepth(SecondNonLinearDepth, SecondNearPlane, SecondFarPlane);
	
	vec4 FirstColor = texture(FirstColorTexture, textureCoords);
    vec4 SecondColor = texture(SecondColorTexture, textureCoords);
	
	if (FirstLinearDepth < SecondLinearDepth)
	{
		outColor = texture(FirstAlbedoTexture, textureCoords);
		gPosition = texture(FirstPositionTexture, textureCoords).xyz;
		gNormal = texture(FirstNormalTexture, textureCoords).xyz;
		gAlbedo = texture(FirstAlbedoTexture, textureCoords);
		gMaterialProperties = texture(FirstMaterialPropertiesTexture, textureCoords);
		gShaderProperties = texture(FirstShaderPropertiesTexture, textureCoords);
		gMotionVectors = texture(FirstMotionVectorsTexture, textureCoords).xy;
		gl_FragDepth = FirstNonLinearDepth;
    }
	else
	{
        outColor = texture(SecondAlbedoTexture, textureCoords);
		gPosition = texture(SecondPositionTexture, textureCoords).xyz;
		gNormal = texture(SecondNormalTexture, textureCoords).xyz;
		gAlbedo = texture(SecondAlbedoTexture, textureCoords);
		gMaterialProperties = texture(SecondMaterialPropertiesTexture, textureCoords);
		gShaderProperties = texture(SecondShaderPropertiesTexture, textureCoords);
		gMotionVectors = texture(SecondMotionVectorsTexture, textureCoords).xy;
        gl_FragDepth = SecondNonLinearDepth;
    }
}