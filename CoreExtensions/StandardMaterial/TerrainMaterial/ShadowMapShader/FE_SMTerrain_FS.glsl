uniform vec3 baseColor;
out vec4 out_Color;

void main(void)
{
	out_Color = vec4(baseColor, 1.0);
}