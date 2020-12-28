@In_Position@
@In_UV@
@In_Material_Index@

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
	fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}