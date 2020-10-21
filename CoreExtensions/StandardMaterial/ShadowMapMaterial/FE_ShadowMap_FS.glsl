#version 450 core
in vec3 fragPosition;
in vec2 UV;
flat in float materialIndex;

@Texture@ baseColorTexture;
@Texture@ baseColorTextureSecondMaterial;
@CameraPosition@

void main(void)
{
	vec4 textureColor = vec4(1.0);
	if (materialIndex == 0.0)
	{
		textureColor = texture(baseColorTexture, UV);
	}
	else if (materialIndex == 1.0)
	{
		textureColor = texture(baseColorTextureSecondMaterial, UV);
	}

	if (textureColor.a < 0.05)
	{
		discard;
	}

	gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}