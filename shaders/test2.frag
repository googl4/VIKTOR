#version 450 core

layout(location = 0) in vec3 vCol;

layout(location = 0) out vec4 colour;

layout(std430, push_constant) uniform constants {
	layout(offset = 64) vec4 lightDir;
};

void main() {
	vec3 n = vCol;
	float l = max( 0, dot( n, -lightDir.xyz ) );
	colour = vec4( l, l, l, 1.0 );
}
