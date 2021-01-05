in vec3 fragPosition;
in vec2 UV;
in flat float materialIndex;

uniform vec3 baseColor;
@MaterialTextures@
@CameraPosition@

vec4 getAlbedo()
{
	vec4 result = vec4(0);
	if (materialIndex == 0.0)
	{
		result = texture(textures[textureBindings[0]], UV);
	}
	else if (materialIndex == 1.0)
	{
		result = texture(textures[textureBindings[6]], UV);
	}

	return result;
}

void main(void)
{
	vec4 textureColor = getAlbedo();
	if (textureColor.a < 0.2)
	{
		discard;
	}

	//gl_FragColor = vec4(1.0f, 0.25f, 0.0f, 1.0f);
	gl_FragColor = vec4(baseColor, 1.0);
}