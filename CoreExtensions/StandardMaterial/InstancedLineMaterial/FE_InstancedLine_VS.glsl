#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 pointA;
layout (location = 2) in vec3 pointB;
layout (location = 3) in vec3 color;
layout (location = 4) in float width;

uniform vec2 resolution;

@ViewMatrix@
@ProjectionMatrix@

out vec3 lineColor;

void main()
{
	lineColor = color;

	vec4 clip0 = FEProjectionMatrix * FEViewMatrix * vec4(pointA, 1.0);
	vec4 clip1 = FEProjectionMatrix * FEViewMatrix * vec4(pointB, 1.0);

	vec2 screen0 = resolution * (0.5 * clip0.xy/clip0.w + 0.5);
	vec2 screen1 = resolution * (0.5 * clip1.xy/clip1.w + 0.5);
	vec2 xBasis = normalize(screen1 - screen0);
	vec2 yBasis = vec2(-xBasis.y, xBasis.x);

	vec2 pt0 = screen0 + (width * 10.0) * (position.x * xBasis + position.y * yBasis);
	vec2 pt1 = screen1 + (width * 10.0) * (position.x * xBasis + position.y * yBasis);
	vec2 pt = mix(pt0, pt1, position.z);
	vec4 clip = mix(clip0, clip1, position.z);
	gl_Position = vec4(clip.w * (2.0 * pt/resolution - 1.0), clip.z, clip.w);
}