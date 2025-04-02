in vec2 textureCoords;

@Texture@ FirstColorTexture;
@Texture@ SecondColorTexture;

@Texture@ FirstDepthTexture;
@Texture@ SecondDepthTexture;

uniform float FirstNearPlane;
uniform float FirstFarPlane;
uniform float SecondNearPlane;
uniform float SecondFarPlane;

out vec4 FinalColor;

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
	
	vec4 ResultColor;
    float ResultDepth;
	
	if (FirstLinearDepth <= SecondLinearDepth)
	{
        // First scene is in front
        ResultColor = FirstColor;
        ResultDepth = FirstNonLinearDepth;
    }
	else
	{
        // Second scene is in front
        ResultColor = SecondColor;
        ResultDepth = SecondNonLinearDepth;
    }
	
	FinalColor = ResultColor;
    gl_FragDepth = ResultDepth;
}