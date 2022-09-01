#include "types.h"
#include "matrix.h"

vec3 intersectTri( vec3 origin, vec3 dir, vec3 v0, vec3 v1, vec3 v2 ) {
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
	return (vec3){ t, u, v };
}

vec2 intersectBox( vec3 origin, vec3 dir, vec3 boxPos, vec3 boxSize, vec3* normal ) {
	vec3 roo = ( origin - boxPos );
	vec3 rdd = dir;
	vec3 m = 1.0f / rdd;
	vec3 n = m * roo;
	vec3 k = vabs( m ) * boxSize;
	vec3 t1 = -n - k;
	vec3 t2 = -n + k;
	float tN = max( max( t1.x, t1.y ), t1.z );
	float tF = min( min( t2.x, t2.y ), t2.z );
	if( tN > tF || tF < 0.0f ) {
		return (vec2){ -1.0f, -1.0f };
	}
	*normal = -vsign( rdd ) * vstep( t1.yzx, t1.xyz ) * vstep( t1.zxy, t1.xyz );
	return (vec2){ tN, tF };
}

int triInBox( int tri, vec3 pos, vec3 size ) {
	vec3 v0 = verts[faces[tri*3+0]];
	vec3 v1 = verts[faces[tri*3+1]];
	vec3 v2 = verts[faces[tri*3+2]];
	
	vec3 tmn = vmin( vmin( v0, v1 ), v2 );
	vec3 tmx = vmax( vmax( v0, v1 ), v2 );
	
	vec3 bmn = pos - size;
	vec3 bmx = pos + size;
	
	if( ( tmn.x <= bmx.x && tmx.x >= bmn.x ) &&
		( tmn.y <= bmx.y && tmx.y >= bmn.y ) &&
		( tmn.z <= bmx.z && tmx.z >= bmn.z )
	) {
		// TODO
		return TRUE;
	}
	
	return FALSE;
}



#define NODE_BRANCH 0
#define NODE_LEAF 1

typedef struct {
	vec3 pos, size;
	u32 children[2];
	u8 nodeType;
	int* tris;
	int numTris;
} bvhNode_t;

/*
bvhNode_t bvh[1024];
int bvhEntries = 0;

int bvhSplitThreshold = 256;
int bvhMaxDepth = 9;
*/

typedef struct {
	bvhNode_t* nodes;
	int entries;
	int maxEntries;
	int splitThreshold;
	int maxDepth;
	
	vec3* verts;
	int vertStride;
	int vertOffset;
} bvh_t;

void splitBVH( int n, int depth ) {
	int splitAxis = bvh[n].size.x > bvh[n].size.y ? 0 : 1;
	splitAxis = bvh[n].size[splitAxis] > bvh[n].size.z ? splitAxis : 2;
	
	int a = bvhEntries++;
	int b = bvhEntries++;
	
	bvh[n].nodeType = NODE_BRANCH;
	bvh[n].children[0] = a;
	bvh[n].children[1] = b;
	
	bvh[a] = bvh[n];
	bvh[b] = bvh[n];
	
	bvh[a].pos[splitAxis] -= bvh[n].size[splitAxis] / 2.0f;
	bvh[a].size[splitAxis] /= 2.0f;
	
	bvh[b].pos[splitAxis] += bvh[n].size[splitAxis] / 2.0f;
	bvh[b].size[splitAxis] /= 2.0f;
	
	int c[2] = { a, b };
	
	for( int i = 0; i < 2; i++ ) {
		bvh[c[i]].nodeType = NODE_LEAF;
		bvh[c[i]].children[0] = -1;
		bvh[c[i]].children[1] = -1;
		
		bvh[c[i]].numTris = 0;
		bvh[c[i]].tris = malloc( sizeof( int ) * bvhSplitThreshold );
		int mtris = bvhSplitThreshold;
		
		for( int j = 0; j < bvh[n].numTris; j++ ) {
			if( triInBox( bvh[n].tris[j], bvh[c[i]].pos, bvh[c[i]].size ) ) {
				if( bvh[c[i]].numTris == mtris ) {
					mtris *= 2;
					bvh[c[i]].tris = realloc( bvh[c[i]].tris, sizeof( int ) * mtris );
					
				}
				
				bvh[c[i]].tris[bvh[c[i]].numTris] = bvh[n].tris[j];
				bvh[c[i]].numTris++;
			}
		}
		
		printf( "bvh %d tris: %d\n", c[i], bvh[c[i]].numTris );
		fflush( stdout );
		
		if( bvh[c[i]].numTris > bvhSplitThreshold && depth < bvhMaxDepth ) {
			splitBVH( c[i], depth + 1 );
		}
	}
}

void initBVH( bvh_t* bvh, int maxDepth, int splitThreshold ) {
	bvh->entries = 0;
	bvh->maxEntries = ( 1 << maxDepth ) * 2;
	bvh->splitThreshold = splitThreshold;
	bvh->maxDepth = maxDepth;
	bvh->nodes = malloc( bvh->maxEntries * sizeof( bvhNode_t ) );
	
	vec3 mn = (vec3){ FLT_MAX, FLT_MAX, FLT_MAX };
	vec3 mx = (vec3){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	
	for( int i = 0; i < numVerts; i++ ) {
		mn = vmin( mn, verts[i] );
		mx = vmax( mx, verts[i] );
	}
	
	bvh[0].nodeType = NODE_BRANCH;
	bvh[0].pos = ( mn + mx ) / 2;
	bvh[0].size = ( mx - mn ) / 2;
	bvh[0].tris = NULL;
	bvh[0].numTris = 0;
	bvh[0].children[0] = 1;
	bvh[0].children[1] = 2;
	
	int splitAxis = bvh[0].size.x > bvh[0].size.y ? 0 : 1;
	splitAxis = bvh[0].size[splitAxis] > bvh[0].size.z ? splitAxis : 2;
	
	bvh[1] = bvh[0];
	bvh[2] = bvh[0];
	
	bvh[1].pos[splitAxis] -= bvh[0].size[splitAxis] / 2.0f;
	bvh[1].size[splitAxis] /= 2.0f;
	
	bvh[2].pos[splitAxis] += bvh[0].size[splitAxis] / 2.0f;
	bvh[2].size[splitAxis] /= 2.0f;
	
	bvhEntries = 3;
	
	for( int n = 1; n < 3; n++ ) {
		bvh[n].nodeType = NODE_LEAF;
		bvh[n].children[0] = -1;
		bvh[n].children[1] = -1;
		
		bvh[n].numTris = 0;
		bvh[n].tris = malloc( sizeof( int ) * bvhSplitThreshold );
		int mtris = bvhSplitThreshold;
		
		for( int i = 0; i < numFaces; i++ ) {
			if( triInBox( i, bvh[n].pos, bvh[n].size ) ) {
				if( bvh[n].numTris == mtris ) {
					mtris *= 2;
					bvh[n].tris = realloc( bvh[n].tris, sizeof( int ) * mtris );
					
				}
				
				bvh[n].tris[bvh[n].numTris] = i;
				bvh[n].numTris++;
			}
		}
		
		printf( "bvh %d tris: %d\n", n, bvh[n].numTris );
		fflush( stdout );
		
		if( bvh[n].numTris > bvhSplitThreshold ) {
			splitBVH( n, 1 );
		}
	}
	
	
}



float traceRayBVH( int n, float preZ, vec3 origin, vec3 dir, vec3* col, vec3* normal ) {
	if( bvh[n].nodeType == NODE_LEAF ) {
		
		vec3 c = { 0, 0, 0 };
		vec3 nrm = { 0, 0, 0 };
		float z = preZ;
		
		for( int i = 0; i < bvh[n].numTris; i++ ) {
			vec3 v0 = verts[faces[bvh[n].tris[i]*3+0]];//-1];
			vec3 v1 = verts[faces[bvh[n].tris[i]*3+1]];//-1];
			vec3 v2 = verts[faces[bvh[n].tris[i]*3+2]];//-1];
			//printf( "ray test {%f,%f,%f}, {%f,%f,%f}, {%f,%f,%f}\n", v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z );
			vec3 t = intersectTri( origin, dir, v0, v1, v2 );
			if( t.x > 0.0f && t.x < z ) {
				//col = tigrRGB( t.y * 255, t.z * 255, ( 1.0f - ( t.y + t.z ) ) * 255 );
				c = (vec3){ t.y, t.z, 1.0f - ( t.y + t.z ) };
				nrm = normalize( cross( v0 - v1, v0 - v2 ) );
				z = t.x;
			}
		}
		
		*col = c;
		*normal = nrm;
		
		return z;
		
		/*
		vec3 bvhN;
		float bvhZ = intersectBox( origin, dir, bvh[n].pos, bvh[n].size, &bvhN );
		
		*normal = bvhN;
		return bvhZ;
		*/
		
	} else {
		int c1 = bvh[n].children[0];
		int c2 = bvh[n].children[1];
		
		vec3 bvhN;
		float bvhZ1 = intersectBox( origin, dir, bvh[c1].pos, bvh[c1].size, &bvhN ).y;
		float bvhZ2 = intersectBox( origin, dir, bvh[c2].pos, bvh[c2].size, &bvhN ).y;
		
		vec3 bvhC1, bvhN1;
		vec3 bvhC2, bvhN2;
		
		if( bvhZ1 >= 0.0f ) {
			bvhZ1 = traceRayBVH( c1, preZ, origin, dir, &bvhC1, &bvhN1 );
		}
		
		if( bvhZ2 >= 0.0f ) {
			bvhZ2 = traceRayBVH( c2, preZ, origin, dir, &bvhC2, &bvhN2 );
		}
		
		int c1Hit = ( bvhZ1 >= 0.0f );
		int c2Hit = ( bvhZ2 >= 0.0f );
		
		if( c1Hit && bvhZ1 < preZ && ( !c2Hit || bvhZ2 > bvhZ1 ) ) {
			*col = bvhC1;
			*normal = bvhN1;
			return bvhZ1;
		}
		
		if( c2Hit && bvhZ2 < preZ ) {
			*col = bvhC2;
			*normal = bvhN2;
			return bvhZ2;
		}
	}
	
	return FLT_MAX;
}

float traceRay( vec3 origin, vec3 dir, vec3* col, vec3* normal ) {
	return traceRayBVH( 0, FLT_MAX, origin, dir, col, normal );
}
