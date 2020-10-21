#version 450 core

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 colour;

layout(push_constant) uniform constants {
	layout(offset = 64) vec4 lightDir;
	layout(offset = 80) uint tIdx;
};

layout(set = 0, binding = 0) uniform texture2D tex[64];
layout(set = 0, binding = 1) uniform sampler s;

void main() {
	colour = texture( sampler2D( tex[tIdx], s ), uv );
}
