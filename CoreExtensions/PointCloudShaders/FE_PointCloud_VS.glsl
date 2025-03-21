@In_Position@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

layout (location = 1) in vec4 Color;
uniform vec3 FEGlobalColorOverride;
uniform bool bUseGlobalColorOverride;

out vec4 VS_OutColor;

void main()
{
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition.x, FEPosition.y, FEPosition.z, 1.0);
	
	if (bUseGlobalColorOverride)
	{
		VS_OutColor = vec4(FEGlobalColorOverride, 1.0);
	}
	else
	{
		VS_OutColor = Color;
	}
}