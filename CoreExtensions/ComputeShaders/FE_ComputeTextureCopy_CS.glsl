layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(binding = 0) uniform sampler2D inImage;
layout(r32f, binding = 1) uniform image2D imgOutput;

uniform vec2 textureSize;

void main()
{
	if (gl_GlobalInvocationID.x >= textureSize.x)
		return;
		
	if (gl_GlobalInvocationID.y >= textureSize.y)
		return;
		
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	float depth = texture(inImage, (texelCoord + vec2(0.5)) / textureSize).r;
	
    imageStore(imgOutput, texelCoord, vec4(depth, depth, 0.0, 1.0));
}