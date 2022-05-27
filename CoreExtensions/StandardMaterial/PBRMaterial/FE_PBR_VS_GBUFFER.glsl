@In_Position@
@In_UV@
@In_Normal@
@In_Tangent@
@In_Material_Index@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

uniform	float FETiling;

#define MAX_LIGHTS 10
out VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	float materialIndex;
} vs_out;

void main(void)
{
	vs_out.UV = FETexCoord * FETiling;

	vec3 T = normalize(vec3(FEWorldMatrix * vec4(FETangent, 0.0)));
	vec3 N = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));
	vs_out.vertexNormal = N;

	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);

	vec4 worldPosition = FEWorldMatrix * vec4(FEPosition, 1.0);
	vs_out.worldPosition = worldPosition.xyz;
	vs_out.viewPosition = FEViewMatrix * worldPosition;
	
	
	gl_Position = vec4(FEPosition, 1.0);
	vs_out.UV = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
	
	//gl_Position = FEProjectionMatrix * vs_out.viewPosition;

	vs_out.materialIndex = FEMatIndex;
}