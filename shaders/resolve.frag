#version 460 core

//layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 colour;

//layout(push_constant) uniform constants {
//	int frame;
//;

layout(set = 0, binding = 0) uniform texture2DMS tex;
layout(set = 0, binding = 1) uniform texture2DMS depthTex;
layout(set = 0, binding = 2) uniform sampler s;

vec3 differentialBlend( vec3 cl, vec3 cr, vec3 cu, vec3 cd ) {
	vec3 dxc = abs( cl - cr );
	vec3 dyc = abs( cu - cd );
	float dx = dxc.r + dxc.g + dxc.b;
	float dy = dyc.r + dyc.g + dyc.b;
	float dd = abs( dx - dy );
	
	vec3 c;
	
	//c = ( cl + cr + cu + cd ) / 4;
	
	
	if( dd < 0.1 ) {
		c = ( cl + cr + cu + cd ) / 4;
		//c = vec3( 1, 0, 0 );
		
	} else {
		if( dx < dy ) {
			c = ( cl + cr ) / 2;
			//c = vec3( 0, 1, 0 );
			
		} else {
			c = ( cu + cd ) / 2;
			//c = vec3( 0, 0, 1 );
		}
	}
	
	
	return c;
}

void main() {
	ivec2 iUV = ivec2( floor( gl_FragCoord.xy ) );
	ivec2 tiUV = iUV / 2;
	
	vec3 c;
	
	
	// 16 VGPRs, 44 SGPRs, 0x578 instructions
	if( ( iUV.y & 1 ) != 0 ) {
		if( ( iUV.x & 1 ) != 0 ) {
			// BR
			c = texelFetch( sampler2DMS( tex, s ), tiUV, 0 ).rgb;
			
		} else {
			// BL
			vec3 cu = texelFetch( sampler2DMS( tex, s ), tiUV, 1 ).rgb;
			vec3 cr = texelFetch( sampler2DMS( tex, s ), tiUV, 0 ).rgb;
			vec3 cl = texelFetch( sampler2DMS( tex, s ), tiUV + ivec2( -1, 0 ), 0 ).rgb;
			vec3 cd = texelFetch( sampler2DMS( tex, s ), tiUV + ivec2( 0, 1 ), 1 ).rgb;
			
			c = differentialBlend( cl, cr, cu, cd );
		}
		
	} else {
		if( ( iUV.x & 1 ) != 0 ) {
			// TR
			vec3 cl = texelFetch( sampler2DMS( tex, s ), tiUV, 1 ).rgb;
			vec3 cd = texelFetch( sampler2DMS( tex, s ), tiUV, 0 ).rgb;
			vec3 cr = texelFetch( sampler2DMS( tex, s ), tiUV + ivec2( 1, 0 ), 1 ).rgb;
			vec3 cu = texelFetch( sampler2DMS( tex, s ), tiUV + ivec2( 0, -1 ), 0 ).rgb;
			
			c = differentialBlend( cl, cr, cu, cd );
			
		} else {
			// TL
			c = texelFetch( sampler2DMS( tex, s ), tiUV, 1 ).rgb;
		}
	}
	
	/*
	// 18 VGPRs, 36 SGPRs, 0x4E4 instructions
	if( ( ( iUV.x ^ iUV.y ) & 1 ) != 0 ) {
		if( ( iUV.y & 1 ) != 0 ) {
			// BL
			vec3 cu = texelFetch( sampler2DMS( tex, s ), iUV, 0 ).rgb;
			vec3 cr = texelFetch( sampler2DMS( tex, s ), iUV, 1 ).rgb;
			vec3 cl = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( -1, 0 ), 1 ).rgb;
			vec3 cd = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( 0, 1 ), 0 ).rgb;
			
			c = differentialBlend( cl, cr, cu, cd );
			
		} else {
			// TR
			vec3 cl = texelFetch( sampler2DMS( tex, s ), iUV, 0 ).rgb;
			vec3 cd = texelFetch( sampler2DMS( tex, s ), iUV, 1 ).rgb;
			vec3 cr = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( 1, 0 ), 0 ).rgb;
			vec3 cu = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( 0, -1 ), 1 ).rgb;
			
			c = differentialBlend( cl, cr, cu, cd );
		}
		
	} else {
		// TL/BR
		c = texelFetch( sampler2DMS( tex, s ), iUV, iUV.y & 1 ).rgb;
	}
	*/
	/*
	// 28 VGPRs, 36 SGPRs, 0x3FC instructions
	if( ( ( iUV.x ^ iUV.y ) & 1 ) != 0 ) {
		vec3 cl, cr, cu, cd;
		
		if( ( iUV.y & 1 ) != 0 ) {
			// BL
			cu = texelFetch( sampler2DMS( tex, s ), iUV, 0 ).rgb;
			cr = texelFetch( sampler2DMS( tex, s ), iUV, 1 ).rgb;
			cl = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( -1, 0 ), 1 ).rgb;
			cd = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( 0, 1 ), 0 ).rgb;
			
		} else {
			// TR
			cl = texelFetch( sampler2DMS( tex, s ), iUV, 0 ).rgb;
			cd = texelFetch( sampler2DMS( tex, s ), iUV, 1 ).rgb;
			cr = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( 1, 0 ), 0 ).rgb;
			cu = texelFetch( sampler2DMS( tex, s ), iUV + ivec2( 0, -1 ), 1 ).rgb;
		}
		
		c = differentialBlend( cl, cr, cu, cd );
		
	} else {
		// TL/BR
		c = texelFetch( sampler2DMS( tex, s ), iUV, iUV.y & 1 ).rgb;
	}
	*/
	
	c = c / ( c + 1 );
	
	colour = vec4( c, 1 );
}
