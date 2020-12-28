in vec2 textureCoords;
@Texture@ sceneTexture;
uniform float intensity;

void main(void)
{
	float finalIntensity = (intensity / 1000.0) * distance(textureCoords, vec2(0.5));

	gl_FragColor.r = texture(sceneTexture, vec2(textureCoords.x + finalIntensity, textureCoords.y)).r;
	gl_FragColor.g = texture(sceneTexture, vec2(textureCoords.x, textureCoords.y)).g;
	gl_FragColor.b = texture(sceneTexture, vec2(textureCoords.x - finalIntensity, textureCoords.y)).b;
}