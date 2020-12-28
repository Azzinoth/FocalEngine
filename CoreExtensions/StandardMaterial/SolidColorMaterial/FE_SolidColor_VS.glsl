@In_Position@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 normal;
out vec3 fragPosition;

void main(void)
{
	normal = normalize(mat3(transpose(inverse(FEWorldMatrix))) * FENormal);
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));

	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}