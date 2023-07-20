layout (local_size_x = 64) in;

@ViewMatrix@
@ProjectionMatrix@
@PVMMatrix@

struct DrawElementsIndirectCommand
{
	uint count;
	uint primCount;
	uint firstIndex;
	uint baseVertex;
	uint baseInstance;
};

#define LOD_COUNT 4

layout(std430, binding = 0) readonly buffer worldMatricesBuffer
{
	mat4 worldMatrices[];
};

layout(std430, binding = 1) readonly buffer positionsBuffer
{
	float positions[];
};

layout(std430, binding = 2) readonly buffer frustumBuffer
{
	float frustum[];
};

layout(std430, binding = 3) writeonly buffer resultLOD0Buffer
{
	mat4 inFrustumLOD0[];
};

layout(std430, binding = 4) readonly buffer AABBSizesBuffer
{
	float AABBSizes[];
};

layout(std430, binding = 5) buffer atomicCountersBuffer
{
	uint atomicCounters[];
};

layout(std430, binding = 6) readonly buffer LODInfoBuffer
{
	float LODInfo[];
};

layout(std430, binding = 7) writeonly buffer resultLOD1Buffer
{
	mat4 inFrustumLOD1[];
};

layout(std430, binding = 8) writeonly buffer resultLOD2Buffer
{
	mat4 inFrustumLOD2[];
};

layout(std430, binding = 9) writeonly buffer resultLOD3Buffer
{
	mat4 inFrustumLOD3[];
};

layout(std430, binding = 10) writeonly buffer drawCallsInfoBuffer
{
	DrawElementsIndirectCommand drawCallsInfo[]; // drawCallsInfo[LOD_COUNT]
};

layout(binding = 0) uniform sampler2D depthPyramid;
uniform bool useOcclusionCulling;
uniform vec2 renderTargetSize;
uniform vec2 nearFarPlanes;

mat4 rotateMat(mat4 original, float angle)
{
	float c = cos(angle);
	float s = sin(angle);

	mat4 rotate;
	rotate[0] = vec4(c, 0, s, 0);
	rotate[1] = vec4(0, 1, 0, 0);
	rotate[2] = vec4(-s, 0, c, 0);
	rotate[3] = vec4(0, 0, 0, 1);

	mat4 result = rotate * original;
	return result;
}

mat4 rotateBillboard(mat4 originalMat)
{
	float xDiff = frustum[24] - originalMat[3][0];
	float yDiff = frustum[26] - originalMat[3][2];

	float angle = atan(yDiff, xDiff) + LODInfo[5];

	mat4 result = mat4(1.0);
	result = rotateMat(result, angle);
	result = result * length(originalMat[0]);
	result[3] = originalMat[3];

	return result;
}

// 2D Polyhedral Bounds of a Clipped, Perspective-Projected 3D Sphere. Michael Mara, Morgan McGuire. 2013
bool projectSphere(vec3 C, float r, float znear, float P00, float P11, out vec4 aabb)
{
	if (C.z < r + znear)
		return false;

	vec2 cx = -C.xz;
	vec2 vx = vec2(sqrt(dot(cx, cx) - r * r), r);
	vec2 minx = mat2(vx.x, vx.y, -vx.y, vx.x) * cx;
	vec2 maxx = mat2(vx.x, -vx.y, vx.y, vx.x) * cx;

	vec2 cy = -C.yz;
	vec2 vy = vec2(sqrt(dot(cy, cy) - r * r), r);
	vec2 miny = mat2(vy.x, vy.y, -vy.y, vy.x) * cy;
	vec2 maxy = mat2(vy.x, -vy.y, vy.y, vy.x) * cy;

	aabb = vec4(minx.x / minx.y * P00, miny.x / miny.y * P11, maxx.x / maxx.y * P00, maxy.x / maxy.y * P11);
	aabb = aabb.xwzy * vec4(0.5f, -0.5f, 0.5f, -0.5f) + vec4(0.5f); // clip space -> uv space

	return true;
}

void main() // FEViewMatrix
{
	// I am in doubt that this should work. But test show that it is...
	if (gl_GlobalInvocationID.x == 0)
	{
		atomicExchange(atomicCounters[0], 0);
		atomicExchange(atomicCounters[1], 0);
		atomicExchange(atomicCounters[2], 0);
		atomicExchange(atomicCounters[3], 0);

		atomicExchange(drawCallsInfo[0].primCount, 0);
		atomicExchange(drawCallsInfo[1].primCount, 0);
		atomicExchange(drawCallsInfo[2].primCount, 0);
		atomicExchange(drawCallsInfo[3].primCount, 0);
	}

	// If instanceCount can't be divided by threadCount evenly
	// we would have threads that will try to work on garbage data from buffer 
	// and result of that work could end up in a output(because of threads scheduling).
	// To avoid this issue, we will not do any work if gl_GlobalInvocationID is out of allowed range.
	uint instanceCount = uint(LODInfo[6]);
	if (gl_GlobalInvocationID.x >= instanceCount)
		return;

	uint invocationOffset = gl_GlobalInvocationID.x * 3;
	float manhattanDistance = abs(frustum[24] - positions[invocationOffset]) +
							  abs(frustum[25] - positions[invocationOffset + 1]) +
							  abs(frustum[26] - positions[invocationOffset + 2]);
	
	// If manhattanDistance is greater than culling distance we should not proceed.
	if (manhattanDistance > LODInfo[0])
		return;

	// Frustum culling.
	for (int i = 0; i < 6; i++)
	{
		uint index = i * 4;
		float distanceToPlane = frustum[index] * positions[invocationOffset] +
								frustum[index + 1] * positions[invocationOffset + 1] +
								frustum[index + 2] * positions[invocationOffset + 2] +
								frustum[index + 3];

		if (distanceToPlane <= AABBSizes[gl_GlobalInvocationID.x])
			return;
	}

	// Occlusion culling
	// Based on https://vkguide.dev/docs/gpudriven/compute_culling/
	if (useOcclusionCulling)
	{
		vec3 center = vec3(positions[invocationOffset], positions[invocationOffset + 1], positions[invocationOffset + 2]);
		center = (FEViewMatrix * vec4(center, 1.0)).xyz;
		center.z = -center.z;

		//flip Y because we access depth texture that way
		center.y *= -1;

		// my AABBSizes buffer containe negative sizes.
		float AABBSize = -(AABBSizes[gl_GlobalInvocationID.x] / 2.0f);
		float zNear = nearFarPlanes.x;
		float zFar = nearFarPlanes.y;

		float P00 = FEProjectionMatrix[0][0];
		float P11 = FEProjectionMatrix[1][1];

		vec4 aabb;
		projectSphere(center, AABBSize, zNear, P00, P11, aabb);

		float width = (aabb.z - aabb.x) * renderTargetSize.x;
		float height = (aabb.w - aabb.y) * renderTargetSize.y;
		float level = floor(log2(max(width, height)));

		vec2 texCoord = (aabb.xy + aabb.zw) * 0.5;

		float downScale = float(pow(2.0, level));
		float levelW = renderTargetSize.x / downScale;
		float levelH = renderTargetSize.y / downScale;

		int pixelX = int(levelW * texCoord.x);
		int pixelY = int(levelH * texCoord.y);

		float candidateDepth_0 = texelFetch(depthPyramid, ivec2(pixelX, pixelY), int(level)).x;
		float candidateDepth_1 = texelFetch(depthPyramid, ivec2(pixelX - 1, pixelY), int(level)).x;
		float candidateDepth_2 = texelFetch(depthPyramid, ivec2(pixelX + 1, pixelY), int(level)).x;
		float candidateDepth_3 = texelFetch(depthPyramid, ivec2(pixelX, pixelY - 1), int(level)).x;
		float candidateDepth_4 = texelFetch(depthPyramid, ivec2(pixelX, pixelY + 1), int(level)).x;

		float depth = max(max(max(max(candidateDepth_0, candidateDepth_1), candidateDepth_2), candidateDepth_3), candidateDepth_4);

		// Multiplication by 3.5 is to minimize false occlusions. (Also on line 188 we divided AABBSize by 2)
		// But this is a problem that should be solved more solidly.
		float viewPosition = center.z - AABBSize * 3.5f;

		float objectDepth = (1.0 / viewPosition - 1.0 / zNear) / (1.0 / zFar - 1.0 / zNear);

		// depth could be 0.0 if we try to read pass valid coordinates or pass valid mip level.
		// objectDepth < 1.0 check because of our multiplication trick above.
		if (objectDepth > depth && objectDepth < 1.0  && depth != 0.0)
			return;
	}

	uint BillboardLOD = uint(LODInfo[4]);

	if (manhattanDistance > LODInfo[3])
	{
		uint newCountOfInstances = atomicAdd(atomicCounters[3], 1);
		atomicMax(drawCallsInfo[3].primCount, newCountOfInstances + 1);
			
		inFrustumLOD3[newCountOfInstances] = BillboardLOD == 3 ? rotateBillboard(worldMatrices[gl_GlobalInvocationID.x]) : worldMatrices[gl_GlobalInvocationID.x];
	}
	else if (manhattanDistance > LODInfo[2])
	{
		uint newCountOfInstances = atomicAdd(atomicCounters[2], 1);
		atomicMax(drawCallsInfo[2].primCount, newCountOfInstances + 1);

		inFrustumLOD2[newCountOfInstances] = BillboardLOD == 2 ? rotateBillboard(worldMatrices[gl_GlobalInvocationID.x]) : worldMatrices[gl_GlobalInvocationID.x];
	}
	else if (manhattanDistance > LODInfo[1])
	{
		uint newCountOfInstances = atomicAdd(atomicCounters[1], 1);
		atomicMax(drawCallsInfo[1].primCount, newCountOfInstances + 1);

		inFrustumLOD1[newCountOfInstances] = BillboardLOD == 1 ? rotateBillboard(worldMatrices[gl_GlobalInvocationID.x]) : worldMatrices[gl_GlobalInvocationID.x];
	}
	else
	{
		uint newCountOfInstances = atomicAdd(atomicCounters[0], 1);
		atomicMax(drawCallsInfo[0].primCount, newCountOfInstances + 1);

		inFrustumLOD0[newCountOfInstances] = BillboardLOD == 0 ? rotateBillboard(worldMatrices[gl_GlobalInvocationID.x]) : worldMatrices[gl_GlobalInvocationID.x];
	}
}