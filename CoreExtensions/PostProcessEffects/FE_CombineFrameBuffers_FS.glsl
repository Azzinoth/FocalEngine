in vec2 textureCoords;

@Texture@ SourceColor;
@Texture@ SourceDepth;

@Texture@ TargetColor;
@Texture@ TargetDepth;

out vec4 ColorOut;

void main(void)
{
	float SourceDepthValue = texture(SourceDepth, textureCoords).r;
    float TargetDepthValue = texture(TargetDepth, textureCoords).r;
    
    if (SourceDepthValue < TargetDepthValue)
    {
        ColorOut = texture(SourceColor, textureCoords);
		gl_FragDepth = SourceDepthValue;
    }
    else
    {
        ColorOut = texture(TargetColor, textureCoords);
		gl_FragDepth = TargetDepthValue;
    }
}