in vec3 normal;
in vec3 fragPosition;

out vec4 out_Color;

uniform vec3 baseColor;
@CameraPosition@

void main(void)
{
	vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.2));
	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.15);
	vec3 diffuseColor = diffuseFactor * vec3(2.0, 2.0, 2.0);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 0.8f;

	out_Color = vec4(baseColor * ambientColor * diffuseColor, 1.0f);
}