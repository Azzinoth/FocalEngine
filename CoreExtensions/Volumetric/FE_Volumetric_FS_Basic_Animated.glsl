in vec2 TextureCoordinates;

@WorldMatrix@

uniform mat4 FEInverseViewMatrix;
uniform mat4 FEInverseProjectionMatrix;

@Texture@ FESceneColor;
uniform sampler2D FESceneDepthMap;
@Texture@ TransferFunctionTexture; // 256x1 RGBA lookup table, rgb = color, a = opacity

// Two animation frames.
uniform sampler3D VolumeTexture;
uniform sampler3D VolumeTexture_2;
uniform float InterpolationFactor;

@CameraPosition@

uniform float FENearPlane;
uniform float FEFarPlane;

out vec4 out_Color;

uniform float DataRangeMin;
uniform float DataRangeMax;
uniform float DataRangeMin_2;
uniform float DataRangeMax_2;
uniform int StepCount;
uniform float OpacityScale;

float LinearizeDepth(float NonLinearDepth, float FENearPlane, float FEFarPlane)
{
	float NDCNormalizedDepth = 2.0 * NonLinearDepth - 1.0;
	return 2.0 * FENearPlane * FEFarPlane / (FEFarPlane + FENearPlane - NDCNormalizedDepth * (FEFarPlane - FENearPlane));
}

// Returns (DistanceToBox, DistanceInsideBox). If the ray misses the box, DistanceInsideBox is zero.
vec2 RayBoxDistance(vec3 BoundsMin, vec3 BoundsMax, vec3 RayOrigin, vec3 RayDirection)
{
	// Guard against direction components that are (near) zero.
	const float Epsilon = 1e-5;

	vec3 SafeDirection = RayDirection;
	if (abs(SafeDirection.x) < Epsilon)
		SafeDirection.x = Epsilon;

	if (abs(SafeDirection.y) < Epsilon)
		SafeDirection.y = Epsilon;

	if (abs(SafeDirection.z) < Epsilon)
		SafeDirection.z = Epsilon;

	vec3 T0 = (BoundsMin - RayOrigin) / SafeDirection;
	vec3 T1 = (BoundsMax - RayOrigin) / SafeDirection;
	vec3 TMin = min(T0, T1);
	vec3 TMax = max(T0, T1);

	float DistanceA = max(max(TMin.x, TMin.y), TMin.z);
	float DistanceB = min(min(TMax.x, TMax.y), TMax.z);

	float DistanceToBox = max(0.0, DistanceA);
	float DistanceInsideBox = max(0.0, DistanceB - DistanceToBox);
	return vec2(DistanceToBox, DistanceInsideBox);
}

void main(void)
{
	// Scene color the volume will be composited over.
	vec4 SceneColor = texture(FESceneColor, TextureCoordinates);
	out_Color = SceneColor;

	// Reconstruct the world-space view ray for this pixel.
	vec2 NDC = TextureCoordinates * 2.0 - 1.0;
	vec4 ClipSpacePosition = vec4(NDC.x, NDC.y, -1.0, 1.0);
	vec4 ViewSpacePosition = FEInverseProjectionMatrix * ClipSpacePosition;
	ViewSpacePosition /= ViewSpacePosition.w;
	vec3 WorldSpacePosition = (FEInverseViewMatrix * ViewSpacePosition).xyz;
	vec3 RayDirection = normalize(WorldSpacePosition - FECameraPosition);

	// World-space distance to the nearest solid scene geometry, so the volume is occluded by it.
	float SceneDepth = LinearizeDepth(texture(FESceneDepthMap, TextureCoordinates).r, FENearPlane, FEFarPlane);

	// Move the ray into the volume's local space, where the volume is the unit cube [0, 1]^3.
	// The direction is deliberately NOT normalized, the march parameter then stays in world-space
	// units, which lets us clip the march against SceneDepth directly.
	mat4 InverseWorldMatrix = inverse(FEWorldMatrix);
	vec3 LocalOrigin = (InverseWorldMatrix * vec4(FECameraPosition, 1.0)).xyz;
	vec3 LocalDirection = (InverseWorldMatrix * vec4(RayDirection, 0.0)).xyz;

	vec2 RayBoxIntersection = RayBoxDistance(vec3(0.0), vec3(1.0), LocalOrigin, LocalDirection);
	float DistanceToBox = RayBoxIntersection.x;
	float DistanceInsideBox = RayBoxIntersection.y;

	// Ray misses the volume, or the whole volume is behind solid geometry.
	if (DistanceInsideBox <= 0.0 || DistanceToBox >= SceneDepth)
		return;

	float WorldStep = DistanceInsideBox / float(StepCount);			// world distance advanced per step
	vec3 LocalEntry = LocalOrigin + LocalDirection * DistanceToBox; // first point on the box surface
	vec3 LocalStep = LocalDirection * WorldStep;          			// local-space step vector
	float LocalStepLength = length(LocalStep);                  	// local sample spacing (constant per ray)
	float ReferenceStepLength = 1.0 / float(StepCount);           	// spacing a unit-length traversal would use

	// Clamp the blend factor so out-of-range values can't extrapolate past either frame.
	float BlendFactor = clamp(InterpolationFactor, 0.0, 1.0);

	// Accumulated color in premultiplied-alpha form, composited front-to-back.
	vec4 Accumulated = vec4(0.0);
	
	float FinalDataRangeMin = mix(DataRangeMin, DataRangeMin_2, BlendFactor);
	float FinalDataRangeMax = mix(DataRangeMax, DataRangeMax_2, BlendFactor);

	for (int Step = 0; Step < StepCount; Step++)
	{
		// Stop once we reach solid scene geometry.
		float WorldDistance = DistanceToBox + (float(Step) + 0.5) * WorldStep;
		if (WorldDistance >= SceneDepth)
			break;

		// Sample the scalar field. Local position inside the unit cube == 3D texture coordinate.
		vec3 SamplePosition = LocalEntry + LocalStep * (float(Step) + 0.5);
		// Sample both animation frames and blend the raw scalar values, so the field itself is
		// interpolated in time before the transfer function maps it to color/opacity.
		float RawValueFirst = texture(VolumeTexture, SamplePosition).r;
		float RawValueSecond = texture(VolumeTexture_2, SamplePosition).r;
		float RawValue = mix(RawValueFirst, RawValueSecond, BlendFactor);

		// Normalize against the data range, then run it through the transfer function.
		float NormalizedValue = clamp((RawValue - FinalDataRangeMin) / (FinalDataRangeMax - FinalDataRangeMin), 0.0, 1.0);
		// Look the color and opacity up in the transfer function LUT (level 0, no mip filtering).
		vec4 TransferFunctionSample = textureLod(TransferFunctionTexture, vec2(NormalizedValue, 0.5), 0.0);
		vec3 SampleColor = TransferFunctionSample.rgb;
		float SampleOpacity = TransferFunctionSample.a * OpacityScale;

		// Opacity correction, rescale the opacity for this sample's actual spacing so the image is independent of StepCount.
		float CorrectedOpacity = 1.0 - pow(1.0 - clamp(SampleOpacity, 0.0, 1.0), LocalStepLength / ReferenceStepLength);

		// Front-to-back "over" compositing with premultiplied alpha.
		Accumulated.rgb += (1.0 - Accumulated.a) * CorrectedOpacity * SampleColor;
		Accumulated.a += (1.0 - Accumulated.a) * CorrectedOpacity;

		// Early ray termination once the accumulated color is effectively opaque.
		if (Accumulated.a >= 0.99)
			break;
	}

	// Composite the volume over the scene (Accumulated is premultiplied).
	out_Color = vec4(SceneColor.rgb * (1.0 - Accumulated.a) + Accumulated.rgb, 1.0);
}