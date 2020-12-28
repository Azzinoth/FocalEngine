@In_Position@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 fragPosition;
out vec3 normal;

void main(void)
{
	normal = FENormal;
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}