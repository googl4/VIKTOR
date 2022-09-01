#ifndef _BVH_H
#define _BVH_H

#include "types.h"
#include "matrix.h"

#define NODE_BRANCH 0
#define NODE_LEAF 1

typedef struct {
	mat4 transform;
	
	vec4 pos;
	vec4 size;
	
	u32* idx;
	vec3* vtx;
	
	u32 idxStart;
	u32 faces;
} chunk_t;

typedef struct {
	vec4 pos;
	vec4 size;
	
	u32 children[2];
	u32 type;
} bvhNode_t;

typedef struct {
	chunk_t* chunks;
	bvhNode_t* nodes;
	
	u32 chunkSize;
} bvh_t;

typedef struct {
	u32* idx;
	vec3* vtx;
	u16* mat;
	vec3* nrm;
	vec2* uv;
	
	u32 numVerts;
	u32 numFaces;
} mesh_t;

#define CHUNK_RADIAL 0x00 // minimise absolute distance, best for dynamic objects
#define CHUNK_AXIAL  0x01 // minimise variance on primary axes, best for static objects with pre-baked transform
#define CHUNK_NORMAL 0x10 // bias distance with normal difference

void BVH_init( bvh_t* bvh, u32 chunkSize );
void BVH_destroy( bvh_t* bvh );
void BVH_build( bvh_t* bvh );

mesh_t buildChunkedMesh( mesh_t original, u32 chunkSize, int heuristic );

#endif
