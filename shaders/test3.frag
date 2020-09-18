#version 450 core

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 colour;

layout(set = 0, binding = 0) uniform sampler2D tex;

void main() {
	colour = texture( tex, uv ) * vec4( 0.1, 1.0, 0.1, 1.0 );
}
