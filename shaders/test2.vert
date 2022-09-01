#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec3 vPos;

layout(push_constant) uniform constants {
	mat4 mvp;
};

void main() {
	gl_Position = mvp * vec4( pos, 1.0 );
	vNormal = normal;
	vUV = uv;
	vPos = pos;
}
