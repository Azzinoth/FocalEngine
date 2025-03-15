@In_Position@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

layout (location = 1) in vec4 Color;

out vec4 VS_OutColor;

void main()
{
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition.x, FEPosition.y, FEPosition.z, 1.0);
	VS_OutColor = Color;
}