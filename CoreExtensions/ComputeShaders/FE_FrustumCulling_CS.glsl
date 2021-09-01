layout (local_size_x = 64) in;

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
	DrawElementsIndirectCommand drawCallsInfo[LOD_COUNT];
};

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

void main()
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
	// To avoid this issue, we will not do any work if gl_GlobalInvocationID is put of allowed range.
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