layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r32f, binding = 0) uniform image2D inImage;
layout(r32f, binding = 1) uniform image2D imgOutput;

uniform vec2 textureSize;
uniform int scaleDownBy;

void main()
{
	if (gl_GlobalInvocationID.x >= textureSize.x)
		return;
		
	if (gl_GlobalInvocationID.y >= textureSize.y)
		return;
		
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy * scaleDownBy);
	
	ivec2 coord = ivec2(texelCoord + vec2(1, 0));
	float depth_0 = imageLoad(inImage, coord).x;
	coord = ivec2(texelCoord + vec2(0, 1));
	float depth_1 = imageLoad(inImage, coord).x;
	coord = ivec2(texelCoord + vec2(1, 1));
	float depth_2 = imageLoad(inImage, coord).x;
	coord = ivec2(texelCoord);
	float depth_3 = imageLoad(inImage, coord).x;

	float depth = max(max(max(depth_0, depth_1), depth_2), depth_3);
	
	value.x = depth;
	value.y = depth;
	
    imageStore(imgOutput, texelCoord / scaleDownBy, value);
}