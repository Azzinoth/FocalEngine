layout (quads, fractional_even_spacing) in;

@PVMMatrix@
uniform float hightScale;

@Texture@ heightMap;

in vec2 TC_UV[];

float height(float u, float v)
{
	//float min = -hightScale;
	//float max = hightScale;
	return texture(heightMap, vec2(u,v)).r * 2 * hightScale - hightScale;
	
	//return (texture(heightMap, vec2(u,v)).r * hightScale);
}

void main(void)
{
    vec2 tc1 = mix(TC_UV[0], TC_UV[1], gl_TessCoord.x);
    vec2 tc2 = mix(TC_UV[2], TC_UV[3], gl_TessCoord.x);
    vec2 TES_UV = mix(tc2, tc1, gl_TessCoord.y);
	
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 position = mix(p2, p1, gl_TessCoord.y);
	position.y += height(TES_UV.x, TES_UV.y);

    gl_Position = FEPVMMatrix * position;
}