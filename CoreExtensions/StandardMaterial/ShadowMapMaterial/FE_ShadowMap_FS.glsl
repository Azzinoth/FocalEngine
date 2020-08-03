#version 450 core
in vec3 fragPosition;
in vec2 UV;

@Texture@ baseColorTexture;
@CameraPosition@

void main(void)
{
	vec4 textureColor = texture(baseColorTexture, UV);
	if (textureColor.a < 0.05)
	{
		discard;
	}

	gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}