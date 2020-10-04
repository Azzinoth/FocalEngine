#version 450 core
@In_Position@
@In_UV@
@In_Normal@
@In_Tangent@
@In_Material_Index@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

#define MAX_LIGHTS 10
out VS_OUT
{
	vec2 UV;
	vec3 fragPosition;
	vec3 worldVertexPosition;
	mat3 TBN;
	vec3 vertexNormal;
	float materialIndex;
} vs_out;

void main(void)
{
	vs_out.UV = FETexCoord;

	vec3 T = normalize(vec3(FEWorldMatrix * vec4(FETangent, 0.0)));
	vec3 N = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));
	vs_out.vertexNormal = N;

	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);

	vs_out.fragPosition = vec3(FEWorldMatrix * vec4(FEPosition, 1.0));
	vs_out.worldVertexPosition = (FEWorldMatrix * vec4(FEPosition, 1.0)).xyz;
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);

	vs_out.materialIndex = FEMatIndex;
}