#version 450 core

@In_Position@
@In_UV@
@In_Instance_Data@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 fragPosition;
out vec2 UV;

void main(void)
{
	UV = FETexCoord;
	vec4 finalPosition = FEWorldMatrix * FEInstanceData * vec4(FEPosition, 1.0);
	fragPosition = finalPosition.xyz;
	gl_Position = FEProjectionMatrix * FEViewMatrix * finalPosition;
}