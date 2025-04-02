in vec2 textureCoords;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 gPosition;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gMaterialProperties;
layout (location = 5) out vec4 gShaderProperties;
layout (location = 6) out vec2 gMotionVectors;

@Texture@ GBufferColorTexture;
@Texture@ GBufferPositionTexture;
@Texture@ GBufferNormalTexture;
@Texture@ GBufferAlbedoTexture;
@Texture@ GBufferMaterialPropertiesTexture;
@Texture@ GBufferShaderPropertiesTexture;
@Texture@ GBufferMotionVectorsTexture;
@Texture@ GBufferDepthTexture;

@Texture@ SecondColorTexture;
@Texture@ SecondDepthTexture;

uniform float FirstNearPlane;
uniform float FirstFarPlane;
uniform float SecondNearPlane;
uniform float SecondFarPlane;

uniform vec3 NormalsToWrite;
uniform vec4 MaterialPropertiesToWrite;
uniform vec4 ShaderPropertiesToWrite;
uniform vec2 MotionVectorsToWrite;

float LinearizeDepth(float NonLinearDepth, float NearPlane, float FarPlane)
{
	float NDCNormalizedDepth = 2.0 * NonLinearDepth - 1.0;
	return 2.0 * NearPlane * FarPlane / (FarPlane + NearPlane - NDCNormalizedDepth * (FarPlane - NearPlane));
}

void main()
{
	float GBufferNonLinearDepth = texture(GBufferDepthTexture, textureCoords).r;
	float GBufferLinearDepth = LinearizeDepth(GBufferNonLinearDepth, FirstNearPlane, FirstFarPlane);
	
	float SecondNonLinearDepth = texture(SecondDepthTexture, textureCoords).r;
	float SecondLinearDepth = LinearizeDepth(SecondNonLinearDepth, SecondNearPlane, SecondFarPlane);
	
	vec4 GBufferColor = texture(GBufferColorTexture, textureCoords);
    vec4 SecondColor = texture(SecondColorTexture, textureCoords);
	
	vec4 ResultColor;
    float ResultDepth;
	
	if (GBufferLinearDepth < SecondLinearDepth)
	{
        ResultColor = texture(GBufferAlbedoTexture, textureCoords);
        ResultDepth = GBufferNonLinearDepth;
		gNormal = texture(GBufferNormalTexture, textureCoords).xyz;
		gMaterialProperties = texture(GBufferMaterialPropertiesTexture, textureCoords);
		gShaderProperties = texture(GBufferShaderPropertiesTexture, textureCoords);
		gMotionVectors = texture(GBufferMotionVectorsTexture, textureCoords).xy;
    }
	else
	{
        ResultColor = SecondColor;
        ResultDepth = SecondNonLinearDepth;
		// Since we do not have the full GBuffer as a second source,
		// we will write to other buffers with user-defined info to prevent visual bugs.
		gNormal = NormalsToWrite;
		gMaterialProperties = MaterialPropertiesToWrite;
		gShaderProperties = ShaderPropertiesToWrite;
		gMotionVectors = MotionVectorsToWrite;
    }
	
	gAlbedo = ResultColor;
    gl_FragDepth = ResultDepth;
}