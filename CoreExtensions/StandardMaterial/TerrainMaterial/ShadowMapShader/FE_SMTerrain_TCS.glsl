#version 450 core

layout (vertices = 4) out;

@PVMMatrix@
@CameraPosition@

@Texture@ heightMap;
@Texture@ displacementMap;
uniform float hightScale;
uniform float scaleFactor;
uniform float LODlevel;
uniform vec2 hightMapShift;
//uniform vec2 tileMult;

in vec2 vertex_UV[];
out vec2 TC_UV[];

float height(float u, float v)
{
	return (texture(heightMap,(vec2(u,v) - hightMapShift) / scaleFactor).r * hightScale);// + texture(displacementMap, vec2(u * tileMult[0],v * tileMult[1])).r);
}

// frustum_edge should be 1, but I gave additional 10% for less aggressive culling. 
#define f 1.1
// Checks if a segment is at least partially inside the frustum
bool segmentInFrustum(vec4 p1, vec4 p2)
{
	p1 = p1 / p1.w;
	p2 = p2 / p2.w;

	if (((p1.x < -f || p1.x > f) || (p1.y < -f || p1.y > f) || (p1.z < -f || p1.z > f)) &&
	    ((p2.x < -f || p2.x > f) || (p2.y < -f || p2.y > f) || (p2.z < -f || p2.z > f)))
		return false;
	else
		return true;
}

void main(void)
{
	if (gl_InvocationID == 0)
    {
		vec4 patchPointsPosition[4];
		patchPointsPosition[0] = vec4(gl_in[0].gl_Position.x, height(vertex_UV[0].x, vertex_UV[0].y), gl_in[0].gl_Position.z, 1.0);
		patchPointsPosition[1] = vec4(gl_in[1].gl_Position.x, height(vertex_UV[1].x, vertex_UV[1].y), gl_in[1].gl_Position.z, 1.0);
		patchPointsPosition[2] = vec4(gl_in[2].gl_Position.x, height(vertex_UV[2].x, vertex_UV[2].y), gl_in[2].gl_Position.z, 1.0);
		patchPointsPosition[3] = vec4(gl_in[3].gl_Position.x, height(vertex_UV[3].x, vertex_UV[3].y), gl_in[3].gl_Position.z, 1.0);
		
		patchPointsPosition[0] = FEPVMMatrix * patchPointsPosition[0];
		patchPointsPosition[1] = FEPVMMatrix * patchPointsPosition[1];
		patchPointsPosition[2] = FEPVMMatrix * patchPointsPosition[2];
		patchPointsPosition[3] = FEPVMMatrix * patchPointsPosition[3];

		if (segmentInFrustum(patchPointsPosition[0], patchPointsPosition[0+1]) ||
			segmentInFrustum(patchPointsPosition[0], patchPointsPosition[0+2]) ||
			segmentInFrustum(patchPointsPosition[0+2], patchPointsPosition[0+3]) ||
			segmentInFrustum(patchPointsPosition[0+3], patchPointsPosition[0+1]))
		{
			vec4 p0 = FEPVMMatrix * gl_in[0].gl_Position;
			vec4 p1 = FEPVMMatrix * gl_in[1].gl_Position;
			vec4 p2 = FEPVMMatrix * gl_in[2].gl_Position;
			vec4 p3 = FEPVMMatrix * gl_in[3].gl_Position;

			p0 /= p0.w;
			p1 /= p1.w;
			p2 /= p2.w;
			p3 /= p3.w;
			
			float l0 = length(p2.xy - p0.xy) * LODlevel + 1.0;
			float l1 = length(p3.xy - p2.xy) * LODlevel + 1.0;
			float l2 = length(p3.xy - p1.xy) * LODlevel + 1.0;
			float l3 = length(p1.xy - p0.xy) * LODlevel + 1.0;

			gl_TessLevelOuter[0] = l0;
			gl_TessLevelOuter[1] = l1;
			gl_TessLevelOuter[2] = l2;
			gl_TessLevelOuter[3] = l3;
			gl_TessLevelInner[0] = min(l1, l3);
			gl_TessLevelInner[1] = min(l0, l2);
		}
		else
		{
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelOuter[3] = 0;

			gl_TessLevelInner[0] = 0;
			gl_TessLevelInner[1] = 0;
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	TC_UV[gl_InvocationID] = (vertex_UV[gl_InvocationID] - hightMapShift) / scaleFactor;
}