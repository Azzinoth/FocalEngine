in vec3 Normal;
in vec3 FragmentPosition;

out vec4 out_Color;

uniform vec3 BaseColor;
// FIXME: Work of this uniform is not correct.
uniform float BrightnessFactor;
@CameraPosition@

void main(void)
{
	vec3 LightDirection = normalize(vec3(0.0, 1.0, 0.2));
	float DiffuseFactor = max(dot(Normal, LightDirection), 0.15);
	vec3 DiffuseColor = DiffuseFactor * vec3(2.0, 2.0, 2.0);
	vec3 AmbientColor = vec3(0.55f, 0.73f, 0.87f) * 0.8f;

	out_Color = vec4(BaseColor * AmbientColor * DiffuseColor * BrightnessFactor, 1.0f);
}