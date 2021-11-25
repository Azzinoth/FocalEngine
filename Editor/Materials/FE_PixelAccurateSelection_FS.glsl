in vec3 fragPosition;
in vec2 UV;
in flat float materialIndex;

uniform vec3 baseColor;
@MaterialTextures@
@CameraPosition@

out vec4 out_Color;

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

    out_Color = vec4(baseColor, 1.0);
}