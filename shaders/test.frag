#version 450 core

layout(location = 0) in vec3 vCol;

layout(location = 0) out vec4 colour;

layout(std430, push_constant) uniform constants {
	int numFaces;
};

layout(std430, binding = 1) buffer vtxBuf {
	vec3 vtx[];
};

layout(std430, binding = 2) buffer idxBuf {
	int idx[];
};

//

static inline vec3 intersectTri( vec3 origin, vec3 dir, vec3 v0, vec3 v1, vec3 v2 ) {
	vec3 v10 = v1 - v0;
	vec3 v20 = v2 - v0;
	vec3 rov0 = origin - v0;
	vec3 n = cross( v10, v20 );
	vec3 q = cross( rov0, dir );
	float d = 1.0f / dot( dir, n );
	float u = d * dot( -q, v20 );
	float v = d * dot( q, v10 );
	float t = d * dot( -n, rov0 );
	if( u < 0.0f || u > 1.0f || v < 0.0f || ( u + v ) > 1.0f ) {
		t = -1.0f;
	}
	return vec3( t, u, v );
}

void main() {
	int w = 1280;
	int h = 720;
	
	float fovY = 56.0f;
	
	vec3 camPos = (vec3){ 0.0f, 0.4f, 2.0f };
	vec3 camDir = (vec3){ 0.0f, -0.1f, -1.0f };
	vec3 worldUp = (vec3){ 0.0f, 1.0f, 0.0f };
	
	vec3 forward = normalize( camDir );
	vec3 right = normalize( cross( forward, worldUp ) );
	vec3 up = normalize( cross( right, forward ) );
	
	float ar = (float)w / h;
	float scale = tanf( DEG_TO_RAD( fovY ) * 0.5f );
	
	
	
	float x2 = (float)rx + 0.5f;
	float y2 = (float)ry + 0.5f;
	float u = ( x2 / w ) * 2.0f - 1.0f;
	float v = -( ( y2 / h ) * 2.0f - 1.0f );
	u = u * ar * scale;
	v = v * scale;
	vec3 viewRay = normalize( forward + right * u + up * v );
	
	
	
	vec3 c = { 0, 0, 0 };
	vec3 n = { 0, 0, 0 };
	float z = FLT_MAX;
	
	for( int i = 0; i < numFaces; i++ ) {
		vec3 v0 = verts[faces[i*3+0]];//-1];
		vec3 v1 = verts[faces[i*3+1]];//-1];
		vec3 v2 = verts[faces[i*3+2]];//-1];
		//printf( "ray test {%f,%f,%f}, {%f,%f,%f}, {%f,%f,%f}\n", v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z );
		vec3 t = intersectTri( origin, dir, v0, v1, v2 );
		if( t.x > 0.0f && t.x < z ) {
			//col = tigrRGB( t.y * 255, t.z * 255, ( 1.0f - ( t.y + t.z ) ) * 255 );
			c = (vec3){ t.y, t.z, 1.0f - ( t.y + t.z ) };
			n = normalize( cross( v0 - v1, v0 - v2 ) );
			z = t.x;
		}
	}
	
	
	
	return z;
	
}
