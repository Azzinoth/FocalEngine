#version 450 core

@In_Position@
@In_UV@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 fragPosition;
out vec2 UV;

void main(void)
{
	UV = FETexCoord;
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}