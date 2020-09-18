#version 450 core

vec2 positions[3] = {
	vec2( -1.0, -1.0 ),
	vec2(  3.0, -1.0 ),
	vec2( -1.0,  3.0 )
};

vec2 uvs[3] = {
	vec2(  0.0,  0.0 ),
	vec2(  2.0,  0.0 ),
	vec2(  0.0,  2.0 )
};

layout(location = 0) out vec2 uv;

void main() {
	gl_Position = vec4( positions[gl_VertexIndex], 0.0, 1.0 );
	uv = uvs[gl_VertexIndex];
}
