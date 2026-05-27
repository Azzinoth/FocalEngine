/**
	* Implementation based on:
	* Schütz, M., Kerbl, B., & Wimmer, M. (2021). Rendering point clouds with compute 
	* shaders and vertex order optimization. In Computer Graphics Forum (Vol. 40, No. 4, pp. 115-126).
	* https://doi.org/10.1111/cgf.14345
*/
#extension GL_NV_gpu_shader5 : enable
#extension GL_NV_shader_atomic_int64 : enable

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

uniform vec3 GlobalColorOverride;
uniform bool bUseGlobalColorOverride;

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct NewPixelDepthAndColor
{
    vec3 position;
    uint color;
};

layout (std430, binding = 1) readonly buffer ParticleBuffer
{
    NewPixelDepthAndColor particles[];
};

layout (std430, binding = 2) buffer FrameBufferData
{
    uint64_t FrameBuffer[];
};

uniform int ScreenWidth;
uniform int ScreenHeight;

void main()
{
	vec3 WorldPosition = particles[gl_GlobalInvocationID.x].position;
	WorldPosition = (FEWorldMatrix * vec4(WorldPosition, 1.0)).xyz;

	vec4 ProjectedPosition = FEProjectionMatrix * FEViewMatrix * vec4(WorldPosition, 1.0);
	vec3 NormalizedDeviceCoordinates = ProjectedPosition.xyz / ProjectedPosition.w;

	if (any(greaterThan(NormalizedDeviceCoordinates, vec3(1.0))) || any(lessThan(NormalizedDeviceCoordinates, vec3(-1.0))))
	{
		return;
	}

	uint PointColor = 0;
	if (bUseGlobalColorOverride)
	{
		uint R = uint(GlobalColorOverride.x * 255.0);
		uint G = uint(GlobalColorOverride.y * 255.0);
		uint B = uint(GlobalColorOverride.z * 255.0);
		PointColor = (R << 0) | (G << 8) | (B << 16) | (0xFF << 24);
	}
	else
	{
		PointColor = particles[gl_GlobalInvocationID.x].color;
	}

	// Skip fully transparent points
	uint Alpha = (PointColor >> 24) & 0xFF;
	if (Alpha == 0)
		return;

	NormalizedDeviceCoordinates.x += 1.0;
	NormalizedDeviceCoordinates.y += 1.0;

	uint PixelX = uint(floor(NormalizedDeviceCoordinates.x * ScreenWidth / 2.0));
	uint PixelY = uint(floor(NormalizedDeviceCoordinates.y * ScreenHeight / 2.0));

	uint64_t NewPixelDepthAndColor = floatBitsToUint(ProjectedPosition.w);
	NewPixelDepthAndColor = (NewPixelDepthAndColor << 32) | PointColor;

	uint PixelIndex = PixelX + PixelY * ScreenWidth;

	uint64_t ExistingPixelDepthAndColor = FrameBuffer[PixelIndex];
	// Early out if the depth is greater than the current depth
	if (NewPixelDepthAndColor < ExistingPixelDepthAndColor)
	{
		atomicMin(FrameBuffer[PixelIndex], NewPixelDepthAndColor);
	}
}