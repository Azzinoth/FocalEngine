#extension GL_NV_gpu_shader5 : enable

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image2D OutputColorImage;
layout(r32f, binding = 1) uniform image2D OutputDepthImage;

layout (std430, binding = 2) buffer FrameBufferData
{
    uint64_t FrameBuffer[];
};

uniform int ScreenWidth;
uniform int ScreenHeight;

uniform float NearPlane;
uniform float FarPlane;

void main()
{
    if (gl_GlobalInvocationID.x >= ScreenWidth)
		return;
		
	if (gl_GlobalInvocationID.y >= ScreenHeight)
		return;

	ivec2 PixelCoordinates = ivec2(gl_GlobalInvocationID.xy);
	int PixelIndex = int(PixelCoordinates.x) + int(PixelCoordinates.y) * ScreenWidth;

	uint64_t PixelDepthAndColor = FrameBuffer[PixelIndex];
	uint PackedColorBits = uint(PixelDepthAndColor & 0xFFFFFFFFu);

	vec4 UnpackedColor;
    UnpackedColor.x = float(PackedColorBits & 0x000000ffu) / 255.0f;
    UnpackedColor.y = float((PackedColorBits & 0x0000ff00u) >> 8) / 255.0f;
    UnpackedColor.z = float((PackedColorBits & 0x00ff0000u) >> 16) / 255.0f;
    UnpackedColor.w = float(PackedColorBits >> 24) / 255.0f;

	imageStore(OutputColorImage, PixelCoordinates, UnpackedColor);

	// Write the depth information to the 32 bit depth buffer.
	uint PackedDepthBits = uint(PixelDepthAndColor >> 32);
	float LinearDepthValue = uintBitsToFloat(PackedDepthBits);

	// Convert linear depth from custom depth buffer to non-linear depth.
	float NormalizedDeviceDepth = ((FarPlane + NearPlane - (2.0 * NearPlane * FarPlane / LinearDepthValue)) / (FarPlane - NearPlane) + 1.0) / 2.0;
	imageStore(OutputDepthImage, PixelCoordinates, vec4(NormalizedDeviceDepth));

	// Using FarPlane bits as the max depth marker for this custom buffer.
	uint64_t NewPixelDepthAndColor = floatBitsToUint(FarPlane);

	// Clearing color part.
	uint NewPackedColorBits = (0u << 0) | (0u << 8) | (0u << 16) | (0xFF << 24);
    NewPixelDepthAndColor = (NewPixelDepthAndColor << 32) | NewPackedColorBits;

	FrameBuffer[PixelIndex] = NewPixelDepthAndColor;
}