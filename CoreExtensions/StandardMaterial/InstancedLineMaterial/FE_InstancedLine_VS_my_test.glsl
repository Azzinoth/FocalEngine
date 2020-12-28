layout (location = 0) in vec3 position;
layout (location = 1) in vec3 pointA;
layout (location = 2) in vec3 pointB;
layout (location = 3) in vec3 color;
layout (location = 4) in float width;

@ViewMatrix@
@ProjectionMatrix@
@CameraPosition@

out vec3 lineColor;

void main()
{
	lineColor = color;


	//vec4 clipA = FEProjectionMatrix * FEViewMatrix * vec4(pointA, 1.0);
	//vec4 clipB = FEProjectionMatrix * FEViewMatrix * vec4(pointB, 1.0);

	//clipA = 0.5 * (clipA / clipA.w) + 0.5;
	//clipB = 0.5 * (clipB / clipB.w) + 0.5;
	
	//DEBUG@(clipA.x)
	//DEBUG@(clipA.y)
	//DEBUG@(clipA.z)
	
	//DEBUG@(clipB.x)
	//DEBUG@(clipB.y)
	//DEBUG@(clipB.z)

	//vec2 xBasis = clipB.xy - clipA.xy;
	
	//DEBUG@(xBasis.x)
	//DEBUG@(xBasis.y)
	
	//vec2 yBasis = vec2(-xBasis.y, xBasis.x);
	//vec4 tempPoint = vec4(clipA.xy + xBasis * position.x + yBasis * width * position.y, clipA.z, 1.0);
	
	
	//tempPoint = tempPoint * 2.0 - 1.0;
	//gl_Position = tempPoint;



	vec2 pos = position.xy;
	vec2 direction = pointB.xy - pointA.xy;
	vec2 normal = normalize(vec2(-direction.y, direction.x));
	
	float zBasis = pointB.z - pointA.z;
	vec4 tempPoint = vec4(pointA.xy + direction * pos.x + normal * width * pos.y, pointA.z + position.z * zBasis, 1.0);

	gl_Position = FEProjectionMatrix * FEViewMatrix * tempPoint;
}