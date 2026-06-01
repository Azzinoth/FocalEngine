in vec2 TextureCoordinates;

@WorldMatrix@

uniform mat4 invProjectionMatrix;
uniform mat4 invViewMatrix;

@Texture@ quadTexture;
@Texture@ DepthTexture;
uniform sampler3D volumeTexture;

@CameraPosition@

uniform float NearPlane;
uniform float FarPlane;

out vec4 out_Color;

const float DataRangeMin = 0.0;  // scalar value mapped to the LOW end of the colour map
const float DataRangeMax = 1.0;  // scalar value mapped to the HIGH end of the colour map
const int StepCount = 256;  	 // number of samples taken across the volume
const float OpacityScale = 1.0;  // overall density multiplier

// That function should be replaced with programable one.
// blue => cyan => green => yellow => red rainbow.
vec3 ColorTransferFunction(float Value)
{
	float Hue = (1.0 - clamp(Value, 0.0, 1.0)) * 0.6667; // 0.6667 (blue) .. 0.0 (red)
	return clamp(abs(mod(Hue * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
}

// Simplest generic mapping, opacity grows linearly with the scalar value.
float OpacityTransferFunction(float Value)
{
	return clamp(Value, 0.0, 1.0);
}

float LinearizeDepth(float NonLinearDepth, float NearPlane, float FarPlane)
{
	float NDCNormalizedDepth = 2.0 * NonLinearDepth - 1.0;
	return 2.0 * NearPlane * FarPlane / (FarPlane + NearPlane - NDCNormalizedDepth * (FarPlane - NearPlane));
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
	// Scene colour the volume will be composited over.
	vec4 SceneColor = texture(quadTexture, TextureCoordinates);
	out_Color = SceneColor;

	// Reconstruct the world-space view ray for this pixel.
	vec2 NDC = TextureCoordinates * 2.0 - 1.0;
	vec4 ClipSpacePosition = vec4(NDC.x, NDC.y, -1.0, 1.0);
	vec4 ViewSpacePosition = invProjectionMatrix * ClipSpacePosition;
	ViewSpacePosition /= ViewSpacePosition.w;
	vec3 WorldSpacePosition = (invViewMatrix * ViewSpacePosition).xyz;
	vec3 RayDirection = normalize(WorldSpacePosition - FECameraPosition);

	// World-space distance to the nearest solid scene geometry, so the volume is occluded by it.
	float SceneDepth = LinearizeDepth(texture(DepthTexture, TextureCoordinates).r, NearPlane, FarPlane);

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

	// Accumulated colour in premultiplied-alpha form, composited front-to-back.
	vec4 Accumulated = vec4(0.0);

	for (int Step = 0; Step < StepCount; Step++)
	{
		// Stop once we reach solid scene geometry.
		float WorldDistance = DistanceToBox + (float(Step) + 0.5) * WorldStep;
		if (WorldDistance >= SceneDepth)
			break;

		// Sample the scalar field. Local position inside the unit cube == 3D texture coordinate.
		vec3 SamplePosition = LocalEntry + LocalStep * (float(Step) + 0.5);
		float RawValue = texture(volumeTexture, SamplePosition).r;

		// Normalize against the data range, then run it through the transfer function.
		float NormalizedValue = clamp((RawValue - DataRangeMin) / (DataRangeMax - DataRangeMin), 0.0, 1.0);
		vec3 SampleColor = ColorTransferFunction(NormalizedValue);
		float SampleOpacity = OpacityTransferFunction(NormalizedValue) * OpacityScale;

		// Opacity correction, rescale the opacity for this sample's actual spacing so the image is independent of StepCount.
		float CorrectedOpacity = 1.0 - pow(1.0 - clamp(SampleOpacity, 0.0, 1.0), LocalStepLength / ReferenceStepLength);

		// Front-to-back "over" compositing with premultiplied alpha.
		Accumulated.rgb += (1.0 - Accumulated.a) * CorrectedOpacity * SampleColor;
		Accumulated.a += (1.0 - Accumulated.a) * CorrectedOpacity;

		// Early ray termination once the accumulated colour is effectively opaque.
		if (Accumulated.a >= 0.99)
			break;
	}

	// Composite the volume over the scene (Accumulated is premultiplied).
	out_Color = vec4(SceneColor.rgb * (1.0 - Accumulated.a) + Accumulated.rgb, 1.0);
}