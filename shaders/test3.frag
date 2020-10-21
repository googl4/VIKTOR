#version 450 core

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 colour;

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 1) uniform texture2D depthTex;
layout(set = 0, binding = 2) uniform sampler s;

float rand( vec2 co ) {
  return fract( sin( dot( co.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 );
}

float linearDepth( float z ) {
	/*
	const float n = 0.1;
	const float f = 1000.0;
	const float c1 = f / n;
	const float c0 = 1.0 - c1;
	
	return 1.0 / ( c0 * z + c1 );
	*/
	const float n = 0.1;
	return ( n / z ) / 2000.0;
}

void main() {
	float z = texture( sampler2D( depthTex, s ), uv ).r;
	float d = linearDepth( z );
	
	vec3 c = texture( sampler2D( tex, s ), uv ).rgb;
	/*
	const int samples = 4;
	const float dofStrength = 0.05;
	const float dofFocus = 0.1;
	
	float blur = abs( d - dofFocus ) * dofStrength;
	
	colour = vec4( 0 );
	for( int i = 0; i < samples; i++ ) {
		int n = i + 1;
		float r1 = rand( uv * n );
		float r2 = rand( uv * ( n * 2 ) );
		vec2 uv2 = uv + ( vec2( r1, r2 ) - 0.5 ) * blur;
		
		//float z2 = texture( depthTex, uv2 ).r;
		//float d2 = linearDepth( z2 );
		
		//colour += mix( texture( tex, uv2 ), vec4( 0.4 ), d2 );
		colour += texture( tex, uv2 );
	}
	colour /= samples;
	*/
	//colour = mix( colour, vec4( 0.4 ), d );
	/*
	float z = texture( depthTex, uv ).r;
	
	const int samples = 64;
	
	float ao = 0;
	for( int i = 0; i < samples; i++ ) {
		int n = i + 1;
		vec2 uv2 = uv + ( vec2( rand( uv * n ), rand( uv * ( n * 2 ) ) ) - 0.5 ) * 0.02;
		
		float z2 = texture( depthTex, uv2 ).r;
		
		ao += float( z2 < z );
	}
	
	ao = 1.0 - ao / samples;
	ao = min( ao * 2.2, 1.0 );
	
	colour = vec4( ao );
	*/
	colour = vec4( c, 0 );
}
