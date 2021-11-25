in vec2 textureCoords;
@Texture@ sceneTexture;
uniform float intensity;

out vec4 out_Color;

void main(void)
{
	float finalIntensity = (intensity / 1000.0) * distance(textureCoords, vec2(0.5));

	out_Color.r = texture(sceneTexture, vec2(textureCoords.x + finalIntensity, textureCoords.y)).r;
	out_Color.g = texture(sceneTexture, vec2(textureCoords.x, textureCoords.y)).g;
	out_Color.b = texture(sceneTexture, vec2(textureCoords.x - finalIntensity, textureCoords.y)).b;
}