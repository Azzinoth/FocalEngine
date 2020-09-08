#version 450 core

in vec3 fragPosition;
in vec2 UV;

uniform vec3 baseColor;
@Texture@ objectTexture;
@CameraPosition@

void main(void)
{
	vec4 textureColor = texture(objectTexture, UV);
	if (textureColor.a < 0.05)
	{
		discard;
	}

    gl_FragColor = vec4(baseColor, 1.0);
}