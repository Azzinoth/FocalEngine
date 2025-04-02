in vec2 textureCoords;

out vec4 outColor;

@Texture@ FirstColorTexture;
@Texture@ FirstDepthTexture;

@Texture@ SecondColorTexture;
@Texture@ SecondDepthTexture;
uniform bool bSecondDepthMapIsInvaild;

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
	
	vec4 ResultColor;
    float ResultDepth;
	
	if (bSecondDepthMapIsInvaild || FirstLinearDepth < SecondLinearDepth)
	{
        ResultColor = FirstColor;
        ResultDepth = FirstNonLinearDepth;
    }
	else
	{
        ResultColor = SecondColor;
        ResultDepth = SecondNonLinearDepth;
    }
	
	outColor = ResultColor;
    gl_FragDepth = ResultDepth;
}