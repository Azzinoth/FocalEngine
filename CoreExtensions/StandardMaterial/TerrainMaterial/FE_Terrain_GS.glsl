layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

@WorldMatrix@
@ViewMatrix@
@PVMMatrix@
@ProjectionMatrix@

in vec2 TES_UV[];
in vec3 TES_normal[];

out GS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
} gs_out;

mat3 getTBN(vec3 tangent, vec3 normal)
{
	vec3 T = normalize(vec3(FEWorldMatrix * vec4(tangent, 0.0)));
	vec3 N = normalize(vec3(FEWorldMatrix * vec4(normal, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);

	return mat3(T, B, N);
}

void main(void)
{
	vec3 q1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 q2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec2 uv0 = TES_UV[0];
	vec2 uv1 = TES_UV[1];
	vec2 uv2 = TES_UV[2];

	float t1 = uv1.y - uv0.y;
	float t2 = uv2.y - uv0.y;
	vec3 tangent = t1 * q2 - t2 * q1;

	gl_Position = FEPVMMatrix * gl_in[0].gl_Position;
	gs_out.viewPosition = FEViewMatrix * FEWorldMatrix * gl_in[0].gl_Position;
	gs_out.worldPosition = vec3(FEWorldMatrix * vec4(gl_in[0].gl_Position.xyz, 1.0));
	gs_out.UV = TES_UV[0];
	gs_out.TBN = getTBN(tangent, TES_normal[0]);
	gs_out.vertexNormal = normalize(vec3(FEWorldMatrix * vec4(TES_normal[0], 0.0)));
	EmitVertex();

	gl_Position = FEPVMMatrix * gl_in[1].gl_Position;
	gs_out.viewPosition = FEViewMatrix * FEWorldMatrix * gl_in[1].gl_Position;
	gs_out.worldPosition = vec3(FEWorldMatrix * vec4(gl_in[1].gl_Position.xyz, 1.0));
	gs_out.UV = TES_UV[1];
	gs_out.TBN = getTBN(tangent, TES_normal[1]);
	gs_out.vertexNormal = normalize(vec3(FEWorldMatrix * vec4(TES_normal[1], 0.0)));
	EmitVertex();

	gl_Position = FEPVMMatrix * gl_in[2].gl_Position;
	gs_out.viewPosition = FEViewMatrix * FEWorldMatrix * gl_in[2].gl_Position;
	gs_out.worldPosition = vec3(FEWorldMatrix * vec4(gl_in[2].gl_Position.xyz, 1.0));
	gs_out.UV = TES_UV[2];
	gs_out.TBN = getTBN(tangent, TES_normal[2]);
	gs_out.vertexNormal = normalize(vec3(FEWorldMatrix * vec4(TES_normal[2], 0.0)));
	EmitVertex();

    EndPrimitive();
}