#version 450 core
#extension GL_EXT_scalar_block_layout : require
//#extension GL_AMD_gpu_shader_half_float : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_KHR_shader_subgroup_basic : require

const uint tileSize = 16; // 16x16 == 8x8x2 CB
const uint tileMaxLights = 30;

layout(location = 0) sample in vec3 normal;
layout(location = 1) sample in vec2 uv;
layout(location = 2) sample in vec3 pos;

layout(location = 0) out vec4 colour;

layout(push_constant) uniform constants {
	layout(offset = 64) vec3 camPos;
	layout(offset = 76) uint numLights;
	layout(offset = 80) uint tIdx;
	layout(offset = 84) uint lightBufStride;
	//layout(offset = 88) uint stIdx;
	layout(offset = 96) vec3 sunColour;
	layout(offset = 108) float shadowDist1;
	layout(offset = 112) vec3 sunDir;
};

layout(set = 0, binding = 0) uniform texture2D tex[4096];
layout(set = 0, binding = 1) uniform sampler s;

layout(set = 0, binding = 2) uniform texture2DArray shadowTex;
layout(set = 0, binding = 3) uniform sampler shadowSampler;

layout(set = 0, binding = 4) uniform uniformBuf {
	mat4 shadowMat[2];
};

struct tile_t {
	uint numLights;
	uint16_t lights[tileMaxLights];
};

layout(set = 0, binding = 5, std430) buffer lightBuf {
	vec4 lights[];
};

layout(set = 0, binding = 6, std430) buffer lightColBuf {
	vec4 lightCols[];
};

layout(set = 0, binding = 7, std430) buffer tileBuf {
	tile_t tiles[];
};

vec2 pcfDither[64] = {
	vec2(  0.390543, -0.147124 ),
	vec2( -0.197626,  0.977416 ),
	vec2(  0.457219,  0.394153 ),
	vec2(  0.058368, -0.941330 ),
	vec2(  0.087683, -0.988768 ),
	vec2(  0.111126, -0.768222 ),
	vec2( -0.540544, -0.471859 ),
	vec2( -0.699194,  0.257256 ),
	vec2(  0.756099, -0.054597 ),
	vec2(  0.086485, -0.183628 ),
	vec2(  0.874604, -0.028097 ),
	vec2( -0.142143, -0.752401 ),
	vec2( -0.190141, -0.261656 ),
	vec2(  0.669040, -0.036223 ),
	vec2( -0.535176,  0.699227 ),
	vec2(  0.288107,  0.836160 ),
	vec2( -0.049655, -0.445878 ),
	vec2( -0.501464,  0.641635 ),
	vec2( -0.201232, -0.484307 ),
	vec2( -0.452689, -0.360673 ),
	vec2(  0.763134, -0.141011 ),
	vec2(  0.069814,  0.649913 ),
	vec2(  0.093541, -0.037845 ),
	vec2(  0.341940,  0.059056 ),
	vec2( -0.608081,  0.214014 ),
	vec2(  0.190319, -0.229542 ),
	vec2( -0.609010,  0.549142 ),
	vec2( -0.015116, -0.999596 ),
	vec2( -0.606937, -0.794708 ),
	vec2( -0.133049, -0.389600 ),
	vec2(  0.921868,  0.372718 ),
	vec2(  0.847934, -0.529833 ),
	vec2( -0.743389,  0.499847 ),
	vec2( -0.184543,  0.878844 ),
	vec2( -0.992303, -0.117110 ),
	vec2( -0.744352, -0.428427 ),
	vec2(  0.756208, -0.361235 ),
	vec2(  0.145886,  0.628100 ),
	vec2(  0.025803, -0.417438 ),
	vec2(  0.203825,  0.561294 ),
	vec2( -0.998953, -0.007964 ),
	vec2(  0.640330, -0.184195 ),
	vec2( -0.011857, -0.658138 ),
	vec2(  0.341198,  0.544910 ),
	vec2( -0.560409,  0.375259 ),
	vec2( -0.983327, -0.174727 ),
	vec2( -0.143896, -0.982608 ),
	vec2( -0.535952, -0.373235 ),
	vec2( -0.740172, -0.146410 ),
	vec2( -0.324812, -0.582438 ),
	vec2( -0.652086, -0.025918 ),
	vec2(  0.529080, -0.521006 ),
	vec2(  0.553950,  0.485965 ),
	vec2( -0.140469,  0.122690 ),
	vec2(  0.955960,  0.124835 ),
	vec2(  0.332751, -0.183035 ),
	vec2( -0.733493, -0.228705 ),
	vec2( -0.149444, -0.755785 ),
	vec2( -0.094258,  0.056417 ),
	vec2( -0.282279,  0.540059 ),
	vec2(  0.486362, -0.589098 ),
	vec2(  0.568057, -0.052700 ),
	vec2( -0.161232,  0.218592 ),
	vec2(  0.275826, -0.802759 )
};

const int pcfSamples = 4;

void main() {
	vec4 t = texture( sampler2D( tex[tIdx], s ), uv );
	
	vec3 N = normalize( normal );
	vec3 V = normalize( camPos - pos );
	
	uint shadowCascade = uint( gl_FragCoord.z < ( 0.1 / shadowDist1 ) );
	
	vec4 shadowPos = shadowMat[shadowCascade] * vec4( pos, 1.0 );
	shadowPos.xy = shadowPos.xy * 0.5 + 0.5;
	shadowPos.z = max( shadowPos.z, 0 );
	
	float shadowVis = 0;
	
	for( int i = 0; i < pcfSamples; i++ ) {
		vec2 ditherPos = shadowPos.xy + pcfDither[( i * 15 + gl_SubgroupInvocationID * 3 ) % 64] * 0.0005f;
		
		float shadowZ = texture( sampler2DArray( shadowTex, shadowSampler ), vec3( ditherPos, shadowCascade ) ).r;
		shadowVis += float( shadowPos.z < shadowZ ) / pcfSamples;
	}
	
	vec3 sunH = normalize( -sunDir + V );
	
	vec3 l = sunColour * shadowVis * ( max( dot( N, -sunDir ), 0 ) + pow( max( dot( N, sunH ), 0 ), 4 ) );
	
	l += vec3( 0.01 );
	
	uint tileAddress = uint( gl_FragCoord.y / tileSize ) * lightBufStride + uint( gl_FragCoord.x / tileSize );
	uint tileNumLights = tiles[tileAddress].numLights;
	
	for( int i = 0; i < min( tileNumLights, tileMaxLights ); i++ ) {
		uint lightIndex = uint( tiles[tileAddress].lights[i] );
		
		vec3 lv = lights[lightIndex].xyz - pos;
		float ld = length( lv );
		float la = 1.0 / ( 1.0 + ld * ld );
		vec3 L = normalize( lv );
		float diff = max( dot( N, L ), 0 );
		vec3 H = normalize( L + V );
		float spec = pow( max( dot( N, H ), 0 ), 4 );
		l += max( lightCols[lightIndex].rgb * ( ( diff + spec ) * la ) - 0.01, 0 );
	}
	
	vec3 c = t.rgb * l;
	
	//c.r += ( float( tileNumLights ) / tileMaxLights ) * 0.2;
	if( tileNumLights > tileMaxLights ) {
		c.r = 1.0;
	}
	
	//c.g += 0.1 * (  1 - shadowCascade );
	
	//c.rgb = vec3( shadowVis );
	//c.rgb = shadowPos.xyz;
	//c.rg = vec2( shadowVis, shadowCascade );
	
	colour = vec4( c, t.a );
}
