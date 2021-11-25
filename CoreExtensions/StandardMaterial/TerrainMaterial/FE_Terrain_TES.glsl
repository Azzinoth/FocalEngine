layout (quads, fractional_even_spacing) in;

uniform float hightScale;
uniform vec2 tileMult;

@Texture@ heightMap;

in vec2 TC_UV[];
out vec2 TES_UV;
out vec3 TES_normal;

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
    TES_UV = mix(tc2, tc1, gl_TessCoord.y);// / scaleFactor;
	
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 position = mix(p2, p1, gl_TessCoord.y);
	position.y += height(TES_UV.x, TES_UV.y);

	// calculate normal
	float gridSpacing = 1.0;
	vec2 tiledUV = vec2(TES_UV.x * tileMult[0], TES_UV.y * tileMult[1]);
	float delta =  1.0 / (textureSize(heightMap,0).x);// / tileMult[0]

	vec3 deltaX = vec3( 2.0 * gridSpacing,
						height(TES_UV.x + delta, TES_UV.y) - height(TES_UV.x - delta, TES_UV.y) , 
						0.0) ;
				
	vec3 deltaZ = vec3( 0.0, 
						height(TES_UV.x, TES_UV.y + delta) - height(TES_UV.x, TES_UV.y - delta) , 
						2.0 * gridSpacing) ;
						
	TES_UV = tiledUV;
	
	TES_normal = normalize(cross(deltaZ, deltaX));
    gl_Position = position;
}