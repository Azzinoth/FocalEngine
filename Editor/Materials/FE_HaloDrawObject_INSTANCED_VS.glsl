@In_Position@
@In_UV@
@In_Material_Index@
@In_Instance_Data@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 fragPosition;
out vec2 UV;
out float materialIndex;

void main(void)
{
	UV = FETexCoord;
	materialIndex = FEMatIndex;
	vec4 finalPosition = FEInstanceData * vec4(FEPosition, 1.0);
	fragPosition = finalPosition.xyz;
	gl_Position = FEProjectionMatrix * FEViewMatrix * finalPosition;
}