@In_Position@
@In_UV@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec2 TextureCoordinates;

void main(void)
{
	TextureCoordinates = FETexCoord;
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}