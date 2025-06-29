@In_Position@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 Normal;
out vec3 FragmentPosition;

void main(void)
{
	Normal = normalize(mat3(transpose(inverse(FEWorldMatrix))) * FENormal);
	FragmentPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));

	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}