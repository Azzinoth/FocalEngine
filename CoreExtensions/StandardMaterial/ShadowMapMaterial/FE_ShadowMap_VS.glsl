#version 450 core
@In_Position@
@In_UV@

out vec2 UV;

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@
out vec3 fragPosition;

void main(void)
{
	UV = FETexCoord;
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}