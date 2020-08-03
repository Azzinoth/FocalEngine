#version 450 core
@In_Position@
@In_UV@
@In_Normal@
@In_Tangent@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

#define MAX_LIGHTS 10
out VS_OUT
{
	vec3 fragPosition;
} vs_out;

void main(void)
{
	vs_out.fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));

	mat4 onlyRotation = FEViewMatrix;
	onlyRotation[3][0] = 0.0;
	onlyRotation[3][1] = 0.0;
	onlyRotation[3][2] = 0.0;
	gl_Position = FEProjectionMatrix * onlyRotation * FEWorldMatrix * vec4(FEPosition, 1.0);
}