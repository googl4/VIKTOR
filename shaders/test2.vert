#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vCol;

layout(std430, push_constant) uniform constants {
	mat4 mvp;
};

void main() {
	gl_Position = mvp * vec4( pos, 1.0 );
	vCol = normal;//pos.xyz * 0.5 + 0.5;//colours[gl_VertexIndex%3];
}

