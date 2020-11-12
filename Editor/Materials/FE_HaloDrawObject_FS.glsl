#version 450 core

in vec3 fragPosition;
in vec2 UV;

@Texture@ objectTexture;
@CameraPosition@

void main(void)
{
	vec4 textureColor = texture(objectTexture, UV);
	if (textureColor.a < 0.2)
	{
		discard;
	}

	gl_FragColor = vec4(1.0f, 0.25f, 0.0f, 1.0f);
}