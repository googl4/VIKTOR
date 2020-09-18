#version 450 core
/*
#extension GL_EXT_multiview : enable
#extension GL_NVX_multiview_per_view_attributes : enable

vec2 positions[2][3] = {
	{
		vec2(  0.0, -0.5 ),
		vec2(  0.5,  0.5 ),
		vec2( -0.5,  0.5 )
	}, {
		vec2(  0.0,  0.5 ),
		vec2(  0.5, -0.5 ),
		vec2( -0.5, -0.5 )
	}
};
*/

vec2 positions[3] = {
	vec2( -1.0, -1.0 ),
	vec2(  3.0, -1.0 ),
	vec2( -1.0,  3.0 )
};

vec3 colours[3] = {
	//vec3(  1.0,  0.0,  0.0 ),
	//vec3( -1.0,  2.0,  0.0 ),
	//vec3( -1.0,  0.0,  2.0 )
	vec3(  0.0,  0.0,  0.0 ),
	vec3(  2.0,  0.0,  0.0 ),
	vec3(  0.0,  2.0,  0.0 )
};

layout(location = 0) out vec3 vCol;

void main() {
	gl_Position = vec4( positions[gl_VertexIndex], 0.0, 1.0 );
	vCol = colours[gl_VertexIndex];
}
