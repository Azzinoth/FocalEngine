in vec3 fragPosition;
in vec3 normal;
@CameraPosition@

void main(void)
{
	vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.2));
	// diffuse part
	float diffuseFactor = max(dot(normal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseFactor * vec3(5.0, 5.0, 5.0);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 0.2f;

	gl_FragColor = vec4(vec3(0.5f, 0.5f, 0.5f) * ambientColor * diffuseColor, 1.0f);
}