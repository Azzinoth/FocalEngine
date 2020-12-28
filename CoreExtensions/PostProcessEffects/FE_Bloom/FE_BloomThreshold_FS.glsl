in vec2 textureCoords;
@Texture@ inputTexture;
uniform float thresholdBrightness;

void main(void)
{
	vec3 color = texture(inputTexture, textureCoords, 2).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > thresholdBrightness)
	{
		gl_FragColor = vec4(color, 1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}