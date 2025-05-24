in vec2 TextureCoordinates;
//in vec3 WorldPosition;

uniform mat4 invProjectionMatrix; // Add this
uniform mat4 invViewMatrix;     // Add this

@Texture@ quadTexture;
@Texture@ DepthTexture;
uniform sampler3D volumeTexture;

@CameraPosition@
@CameraDirection@

uniform float NearPlane;
uniform float FarPlane;

out vec4 out_Color;

// Adapted from: http://jcgt.org/published/0007/03/04/ and later from a Sebastian Lague youtube channel.
// Returns (dstToBox, dstInsideBox). If ray misses box, dstInsideBox will be zero
vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 rayDir)
{
    vec3 t0 = (boundsMin - rayOrigin) / rayDir;
    vec3 t1 = (boundsMax - rayOrigin) / rayDir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(min(tmax.x, tmax.y), tmax.z);

    // CASE 1: ray intersects box from outside (0 <= dstA <= dstB)
    // dstA is dst to nearest intersection, dstB dst to far intersection

    // CASE 2: ray intersects box from inside (dstA < 0 < dstB)
    // dstA is the dst to intersection behind the ray, dstB is dst to forward intersection

    // CASE 3: ray misses box (dstA > dstB)

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}

float LinearizeDepth(float NonLinearDepth, float NearPlane, float FarPlane)
{
	float NDCNormalizedDepth = 2.0 * NonLinearDepth - 1.0;
	return 2.0 * NearPlane * FarPlane / (FarPlane + NearPlane - NDCNormalizedDepth * (FarPlane - NearPlane));
}

vec3 worldToTexCoords(vec3 worldPos, vec3 boxMin, vec3 boxMax) 
{
    // Normalize position within the box bounds to [0,1] range
    return (worldPos - boxMin) / (boxMax - boxMin);
}

void main(void)
{
	out_Color = texture(quadTexture, TextureCoordinates);
	
	
    vec2 ndc = TextureCoordinates * 2.0 - 1.0;
    vec4 clipSpacePos = vec4(ndc.x, ndc.y, -1.0, 1.0);
    vec4 viewSpacePos = invProjectionMatrix * clipSpacePos;
    viewSpacePos /= viewSpacePos.w;
    vec4 worldSpacePos = invViewMatrix * viewSpacePos;
    vec3 rayDir = normalize(worldSpacePos.xyz - FECameraPosition);
	
	
	// most probably worldSpacePos is not accurate enough ?
	
	float NonLinearDepth = texture(DepthTexture, TextureCoordinates).r;
	float LinearDepth = LinearizeDepth(NonLinearDepth, NearPlane, FarPlane);
	
	
	vec3 BoxMin = vec3(-1.1);
	vec3 BoxMax = vec3(1.1);
	vec2 RayBoxIntersection = rayBoxDst(BoxMin, BoxMax, FECameraPosition, rayDir);
	float DistanceToBox = RayBoxIntersection.x;
	float DistanceInsideBox = RayBoxIntersection.y;
	
	
	
	
	
	float NumSteps = 1500.0;
	float dstTravelled = 0;
	float stepSize = DistanceInsideBox / NumSteps;
	float dstLimit = min(LinearDepth - DistanceToBox, DistanceInsideBox);

	// March through volume:
	float totalDensity = 0;
	while (dstTravelled < dstLimit) {
		vec3 rayPos = FECameraPosition + rayDir * (DistanceToBox + dstTravelled);
		
		vec3 texCoords = worldToTexCoords(rayPos, BoxMin, BoxMax);
		vec4 sampleColor = texture(volumeTexture, texCoords);
		float density = sampleColor.r;
		
		
		//totalDensity += DistanceInsideBox * stepSize; // sampleDensity(rayPos) or 0.05
		totalDensity += density;
		dstTravelled += stepSize;
	}
	
	totalDensity *= 0.05;
	
	float transmittance = exp(-totalDensity);
	
	
	//out_Color += vec4(1.0, 0.0, 0.0, 1.0) * (1.0 - transmittance);
	out_Color *= transmittance;
	
	
	
	
	
	//bool RayHit = DistanceInsideBox > 0.0 && DistanceToBox < LinearDepth;
	//if (RayHit)
	//	out_Color = vec4(transmittance);
}