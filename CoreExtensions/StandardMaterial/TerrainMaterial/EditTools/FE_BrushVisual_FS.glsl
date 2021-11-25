in vec2 textureCoords;
uniform vec2 brushCenter;
uniform float brushSize;

out vec4 out_Color;

void main(void)
{
	float distanceToBrush = distance(brushCenter, textureCoords);
	if (distanceToBrush < brushSize)
	{
		float persentage = (distanceToBrush / brushSize);
		float displacement = 1.0 * (1 - persentage);
		out_Color = vec4(0.0) + displacement;
	}
	else
	{
		out_Color = vec4(0.0);
	}
}