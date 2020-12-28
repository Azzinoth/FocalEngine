in vec2 textureCoords;
uniform vec2 brushCenter;
uniform float brushSize;
	
void main(void)
{
	float distanceToBrush = distance(brushCenter, textureCoords);
	if (distanceToBrush < brushSize)
	{
		float persentage = (distanceToBrush / brushSize);
		float displacement = 1.0 * (1 - persentage);
		gl_FragColor = vec4(0.0) + displacement;
	}
	else
	{
		gl_FragColor = vec4(0.0);
	}

	//gl_FragColor = vec4(1.0);
}