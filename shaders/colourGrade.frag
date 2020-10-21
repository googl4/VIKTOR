#version 460 core
#extension GL_ARB_shader_ballot : require
//#extension GL_AMD_gpu_shader_half_float : require
//#extension GL_AMD_gpu_shader_half_float_fetch : require

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 colour;

layout(push_constant) uniform constants {
	vec3 bias;
	float tonemapStrength;
	vec3 gain;
	float saturation;
	vec3 curve;
	float gamma;
	vec3 primaryColour;
};

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 1) uniform sampler s;

/*
const float dither[4][4] = {
	{  0.0 / 16.0,  8.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0 },
	{ 12.0 / 16.0,  4.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0 },
	{  3.0 / 16.0, 11.0 / 16.0,  1.0 / 16.0,  9.0 / 16.0 },
	{ 15.0 / 16.0,  7.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0 }
};
*/
/*
const float dither[16] = {
	 0.0 / 16.0,  8.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0,
	12.0 / 16.0,  4.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0,
	 3.0 / 16.0, 11.0 / 16.0,  1.0 / 16.0,  9.0 / 16.0,
	15.0 / 16.0,  7.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0
};
*/

const float dither[16] = {
	 0.0 / 16.0,  8.0 / 16.0, 12.0 / 16.0,  4.0 / 16.0,
	 2.0 / 16.0, 10.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0,
	 3.0 / 16.0, 11.0 / 16.0, 15.0 / 16.0,  7.0 / 16.0,
	 1.0 / 16.0,  9.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0
};

void main() {
	vec3 c = texture( sampler2D( tex, s ), uv ).rgb;
	c = pow( c * gain + bias, curve );
	float luma = dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
	c = mix( primaryColour * luma, c, saturation );
	c = c / ( c + tonemapStrength );
	c = pow( c, vec3( gamma ) );
	//colour = vec4( c + dither[int(gl_FragCoord.y)%4][int(gl_FragCoord.x)%4] * ( 1.0 / 256.0 ), 0.0 );
	colour = vec4( c + dither[gl_SubGroupInvocationARB%16] * ( 1.0 / 256.0 ), 0.0 );
}
