@In_Position@

out vec2 TextureCoordinates;
//out vec3 WorldPosition;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	
	TextureCoordinates = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
	//WorldPosition = (FEWorldMatrix * vec4(FEPosition, 1.0)).xyz;
}