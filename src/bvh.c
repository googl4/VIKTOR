#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "types.h"
#include "matrix.h"
#include "array.h"
#include "bvh.h"

int triInBox( vec4 v0, vec4 v1, vec4 v2, vec4 pos, vec4 size ) {
	vec4 tmn = vec4_min( vec4_min( v0, v1 ), v2 );
	vec4 tmx = vec4_max( vec4_max( v0, v1 ), v2 );
	
	vec4 bmn = pos - size;
	vec4 bmx = pos + size;
	
	if( ( tmn[0] <= bmx[0] && tmx[0] >= bmn[0] ) &&
		( tmn[1] <= bmx[1] && tmx[1] >= bmn[1] ) &&
		( tmn[2] <= bmx[2] && tmx[2] >= bmn[2] )
	) {
		return TRUE;
	}
	
	return FALSE;
}

void BVH_init( bvh_t* bvh, u32 chunkSize ) {
	bvh->chunks = arr_new( chunk_t, 16 );
	bvh->nodes = arr_new( bvhNode_t, 16 );
	
	bvh->chunkSize = chunkSize;
	
	/*
	bvhNode_t* n = arr_extend( bvh->nodes );
	n->pos = (vec4){ 0, 0, 0 };
	n->size = (vec4){ FLT_MAX, FLT_MAX, FLT_MAX };
	n->children[0] = -1;
	n->children[1] = -1;
	n->type = NODE_BRANCH;
	*/
}

void BVH_destroy( bvh_t* bvh ) {
	arr_free( bvh->chunks );
	arr_free( bvh->nodes );
}

// void buildChunkedMesh( ...
// void bakeTransform( ...

mesh_t buildChunkedMesh( mesh_t original, u32 chunkSize, int heuristic ) {
	mesh_t new;
	
	new.idx = malloc( original.numFaces * 3 * sizeof( u32 ) );
	new.vtx = malloc( original.numVerts * sizeof( vec3 ) );
	new.mat = malloc( original.numFaces * sizeof( u16 ) );
	new.numVerts = original.numVerts;
	new.numFaces = original.numFaces;
	
	memcpy( new.vtx, original.vtx, original.numVerts * sizeof( vec3 ) );
	
	vec4* faceCentre = malloc( original.numFaces * sizeof( vec4 ) );
	u8* faceProcessed = malloc( original.numFaces );
	
	float* faceDist = malloc( original.numFaces * sizeof( float ) );
	int* faceId = malloc( original.numFaces * sizeof( int ) );
	
	vec4* faceNormal = malloc( original.numFaces * sizeof( vec4 ) );
	
	for( int i = 0; i < original.numFaces; i++ ) {
		u32 i0 = original.idx[i*3+0];
		u32 i1 = original.idx[i*3+1];
		u32 i2 = original.idx[i*3+2];
		
		vec4 v0 = (vec4){ original.vtx[i0][0], original.vtx[i0][1], original.vtx[i0][2] };
		vec4 v1 = (vec4){ original.vtx[i1][0], original.vtx[i1][1], original.vtx[i1][2] };
		vec4 v2 = (vec4){ original.vtx[i2][0], original.vtx[i2][1], original.vtx[i2][2] };
		
		vec4 pos = ( v0 + v1 + v2 ) / 3;
		
		vec4 nrm = vec4_cross( v1 - v0, v2 - v0 );
		
		//printf( "face %d: %f, %f, %f\n", i, pos[0], pos[1], pos[2] );
		
		faceCentre[i] = pos;
		faceNormal[i] = nrm;
		faceProcessed[i] = FALSE;
		faceDist[i] = FLT_MAX;
		faceId[i] = i;
	}
	
	int processedFaces = 0;
	int nextFace = 0;
	
	while( processedFaces < original.numFaces ) {
		int faces = min( original.numFaces - processedFaces, chunkSize );
		
		while( faceProcessed[nextFace] ) {
			nextFace++;
		}
		
		vec4 pos = faceCentre[nextFace];
		vec4 normal = faceNormal[nextFace];
		
		//printf( "chunk start %d: %d  %f, %f, %f\n", nextFace, faces, pos[0], pos[1], pos[2] );
		//fflush( stdout );
		
		for( int j = 0; j < original.numFaces; j++ ) {
			//vec4 v = faceCentre[j];
			u32 i0 = original.idx[j*3+0];
			u32 i1 = original.idx[j*3+1];
			u32 i2 = original.idx[j*3+2];
			
			vec4 v0 = (vec4){ original.vtx[i0][0], original.vtx[i0][1], original.vtx[i0][2] };
			vec4 v1 = (vec4){ original.vtx[i1][0], original.vtx[i1][1], original.vtx[i1][2] };
			vec4 v2 = (vec4){ original.vtx[i2][0], original.vtx[i2][1], original.vtx[i2][2] };
			
			float d = 0;
			
			if( heuristic & 0x0F == CHUNK_RADIAL ) {
				float d0 = vec4_length( pos - v0 );
				float d1 = vec4_length( pos - v1 );
				float d2 = vec4_length( pos - v2 );
				
				d = fmaxf( fmaxf( d0, d1 ), d2 );
				
			} else if( heuristic & 0x0F == CHUNK_AXIAL ) {
				vec4 ad0 = pos - v0;
				vec4 ad1 = pos - v1;
				vec4 ad2 = pos - v2;
				
				d = fabsf( ad0[0] );
				d = fmaxf( d, fabsf( ad0[1] ) );
				d = fmaxf( d, fabsf( ad0[2] ) );
				
				d = fmaxf( d, fabsf( ad1[0] ) );
				d = fmaxf( d, fabsf( ad1[1] ) );
				d = fmaxf( d, fabsf( ad1[2] ) );
				
				d = fmaxf( d, fabsf( ad2[0] ) );
				d = fmaxf( d, fabsf( ad2[1] ) );
				d = fmaxf( d, fabsf( ad2[2] ) );
			}
			
			if( heuristic & CHUNK_NORMAL ) {
				d *= 1.2f - vec4_dot( normal, faceNormal[j] );
			}
			
			faceDist[j] = d;
			faceId[j] = j;
		}
		
		//printf( "updated distances\n" );
		//fflush( stdout );
		
		int gap = ( 1 << ( 32 - __builtin_clz( original.numFaces ) ) ) - 1;
		
		while( gap > 0 ) {
			int sorted = FALSE;
			
			while( !sorted ) {
				sorted = TRUE;
				
				for( int j = gap; j < original.numFaces; j++ ) {
					if( faceDist[j] < faceDist[j - gap] ) {
						sorted = FALSE;
						
						float tmpDist = faceDist[j];
						int tmpId = faceId[j];
						
						faceDist[j] = faceDist[j - gap];
						faceId[j] = faceId[j - gap];
						
						faceDist[j - gap] = tmpDist;
						faceId[j - gap] = tmpId;
					}
				}
			}
			
			gap = ( gap + 1 ) / 2 - 1;
		}
		
		//printf( "sorted distances\n" );
		//fflush( stdout );
		
		int lastFace = 0;
		for( int j = 0; j < faces; j++ ) {
			for( int k = lastFace; k < original.numFaces; k++ ) {
				int f = faceId[k];
				
				if( !faceProcessed[f] /*&& original.mat[f] == original.mat[nextFace]*/ ) {
					new.idx[processedFaces*3+0] = original.idx[f*3+0];
					new.idx[processedFaces*3+1] = original.idx[f*3+1];
					new.idx[processedFaces*3+2] = original.idx[f*3+2];
					
					new.mat[processedFaces] = original.mat[f];
					
					faceProcessed[f] = TRUE;
					processedFaces++;
					
					lastFace = k + 1;
					
					//printf( "face %d\n", f );
					
					break;
				}
			}
		}
		
		//printf( "chunk end %d: %d\n", nextFace, processedFaces );
		//fflush( stdout );
		
		//processedFaces += faces;
		nextFace++;
	}
	
	free( faceCentre );
	free( faceProcessed );
	free( faceDist );
	free( faceId );
	
	return new;
}

/*
void BVH_addMesh( bvh_t* bvh, u32* idx, vec3* vtx, u32 numFaces, mat4 transform ) { // needs spatially coherent index buffer to be effective
	for( u32 i = 0; i < numFaces; i += bvh->chunkSize ) {
		chunk_t* chunk = arr_extend( bvh->chunks );
		
		chunk->idx = idx;
		chunk->vtx = vtx;
		
		chunk->idxStart = i * 3;
		chunk->faces = min( numFaces - i, bvh->chunkSize );
		
		vec4 mn = (vec4){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
		vec4 mx = (vec4){  FLT_MAX,  FLT_MAX,  FLT_MAX };
		
		for( int j = chunk->idxStart; j < chunk->idxStart + chunk->faces * 3; j++ ) {
			vec4 v = *((vec4*)(vtx[idx[j]]));
			
			v.w = 1;
			v = mat4_mul_vec4( transform, v );
			
			mn = vec4_min( mn, v );
			mx = vec4_max( mx, v );
		}
		
		chunk->pos = ( mn + mx ) / 2;
		chunk->size = ( mx - mn ) / 2;
		
		chunk->transform = transform;
	}
}
*/

void BVH_build( bvh_t* bvh ) {
	
}

