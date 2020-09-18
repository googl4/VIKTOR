#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

typedef float vec2 __attribute__((vector_size(  8 )));
typedef float vec4 __attribute__((vector_size( 16 )));
typedef float vec3[3];
typedef vec3 mat3[3];
//typedef vec4 mat4[4];
typedef struct {
	vec4 c[4];
} mat4;

#endif
