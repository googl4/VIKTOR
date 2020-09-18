#ifndef _MATRIX_H
#define _MATRIX_H

#include <math.h>

#include "types.h"

static inline float vec4_dot( vec4 a, vec4 b ) {
	float n = 0;
	
	for( int i = 0; i < 4; i++ ) {
		n += a[i] * b[i];
	}
	
	return n;
}

static inline float vec4_length( vec4 v ) {
	return sqrtf( vec4_dot( v, v ) );
}

static inline vec4 vec4_norm( vec4 v ) {
	return v / vec4_length( v );
}

static inline vec4 vec4_cross( vec4 a, vec4 b ) {
	vec4 r;
	
	r[0] = a[1] * b[2] - a[2] * b[1];
	r[1] = a[2] * b[0] - a[0] * b[2];
	r[2] = a[0] * b[1] - a[1] * b[0];
	r[3] = 1.0f;
	
	return r;
}

static inline vec4 vec4_reflect( vec4 v, vec4 n ) {
	vec4 r;
	
	float p = 2.0f * vec4_dot( v, n );
	
	for( int i = 0; i < 4; i++ ) {
		r[i] = v[i] - p * n[i];
	}
	
	return r;
}

static inline mat4 mat4_identity( void ) {
	mat4 m = { {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	} };
	
	return m;
}

static inline vec4 mat4_row( mat4 m, int n ) {
	vec4 r;
	
	for( int i = 0; i < 4; i++ ) {
		r[i] = m.c[i][n];
	}
	
	return r;
}

static inline vec4 mat4_col( mat4 m, int n ) {
	return m.c[n];
}

static inline mat4 mat4_scale( mat4 m, float s ) {
	for( int i = 0; i < 4; i++ ) {
		m.c[i] *= s;
	}
	
	return m;
}

static inline mat4 mat4_scale_aniso( mat4 m, vec4 s ) {
	for( int i = 0; i < 4; i++ ) {
		m.c[i] *= s[i];
	}
	
	return m;
}

/*
LINMATH_H_FUNC void mat4x4_scale_aniso(mat4x4 M, mat4x4 a, float x, float y, float z)
{
	int i;
	vec4_scale(M[0], a[0], x);
	vec4_scale(M[1], a[1], y);
	vec4_scale(M[2], a[2], z);
	for(i = 0; i < 4; ++i) {
		M[3][i] = a[3][i];
	}
}
*/

static inline mat4 mat4_mul( mat4 a, mat4 b ) {
	mat4 r;
	
	for( int i = 0; i < 4; i++ ) {
		for( int j = 0; j < 4; j++ ) {
			r.c[i][j] = 0;
			for( int k = 0; k < 4; k++ ) {
				r.c[i][j] += a.c[k][j] * b.c[i][k];
			}
		}
	}
	
	return r;
}

static inline vec4 mat4_mul_vec4( mat4 m, vec4 v ) {
	vec4 r;
	
	for( int i = 0; i < 4; i++ ) {
		r[i] = 0;
		for( int j = 0; j < 4; j++ ) {
			r[i] += m.c[j][i] * v[j];
		}
	}
	
	return r;
}

static inline mat4 mat4_translate( vec4 t ) {
	mat4 m = mat4_identity();
	
	m.c[3][0] = t[0];
	m.c[3][1] = t[1];
	m.c[3][2] = t[2];
	
	return m;
}

static inline mat4 mat4_translate_in_place( mat4 m, vec4 t ) {
	for( int i = 0; i < 4; i++ ) {
		vec4 r = mat4_row( m, i );
		m.c[3][i] += vec4_dot( r, t );
	}
	
	return m;
}

static inline mat4 mat4_perspective( float fov, float aspect, float n, float f ) {
	mat4 m = {};
	
	float a = 1.0f / tanf( fov / 2.0f );
	
	m.c[0][0] = a / aspect;
	m.c[1][1] = a;
	m.c[2][2] = -( ( f + n ) / ( f - n ) );
	m.c[2][3] = -1.0f;
	m.c[3][2] = -( ( 2.0f * f * n ) / ( f - n ) );
	
	return m;
}

static inline mat4 mat4_lookat( vec4 eye, vec4 centre, vec4 up ) {
	mat4 m = {};
	
	vec4 f = vec4_norm( centre - eye );
	vec4 s = vec4_norm( vec4_cross( f, up ) );
	vec4 t = vec4_cross( s, f );
	
	m.c[0][0] = s[0];
	m.c[0][1] = t[0];
	m.c[0][2] = -f[0];
	
	m.c[1][0] = s[1];
	m.c[1][1] = t[1];
	m.c[1][2] = -f[1];
	
	m.c[2][0] = s[2];
	m.c[2][1] = t[2];
	m.c[2][2] = -f[2];
	
	m.c[3][3] = 1.0f;
	
	m = mat4_translate_in_place( m, -eye );
	
	return m;
}

static inline vec4 quat_identity( void ) {
	vec4 q = { 0, 0, 0, 1 };
	
	return q;
}

static inline vec4 quat_mul( vec4 a, vec4 b ) {
	vec4 r = vec4_cross( a, b ) + a * b[3] + b * a[3];
	
	r[3] = a[3] * b[3] - vec4_dot( a, b );
	
	return r;
}

static inline vec4 quat_rotate( vec4 axis, float angle ) {
	vec4 r = axis * sinf( angle / 2.0f );
	
	r[3] = cosf( angle / 2.0f );
	
	return r;
}

static inline mat4 mat4_from_quat( vec4 q ) {
	mat4 m = {};
	
	float a = q[3];
	float b = q[0];
	float c = q[1];
	float d = q[2];
	float a2 = a * a;
	float b2 = b * b;
	float c2 = c * c;
	float d2 = d * d;
	
	m.c[0][0] = a2 + b2 - c2 - d2;
	m.c[0][1] = 2.0f * ( b * c + a * d );
	m.c[0][2] = 2.0f * ( b * d - a * c );
	
	m.c[1][0] = 2.0f * ( b * c - a * d );
	m.c[1][1] = a2 - b2 + c2 - d2;
	m.c[1][2] = 2.0f * ( c * d + a * b );
	
	m.c[2][0] = 2.0f * ( b * d + a * c );
	m.c[2][1] = 2.0f * ( c * d - a * b );
	m.c[2][2] = a2 - b2 - c2 + d2;
	
	m.c[3][3] = 1.0f;
	
	return m;
}

#endif
