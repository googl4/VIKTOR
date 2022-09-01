#ifndef _ARRAY_H
#define _ARRAY_H

static void* arr_new_internal( size_t preAlloc, size_t elementSize ) {
	size_t* p = malloc( sizeof( size_t ) * 2 + preAlloc * elementSize );
	p[0] = preAlloc;
	p[1] = 0;
	return p + 2;
}

#define arr_new( type, preAlloc ) arr_new_internal( preAlloc, sizeof( type ) )

static void* arr_head_ptr_internal( void* arr, size_t elementSize ) {
	size_t len = ((size_t*)arr)[-1];
	return (char*)arr + len * elementSize;
}

static void* arr_alloc_head_internal( void* arr, size_t elementSize ) {
	size_t len = ((size_t*)arr)[-1];
	size_t maxLen = ((size_t*)arr)[-2];
	
	if( len == maxLen ) {
		maxLen *= 2;
		
		size_t* basePtr = (size_t*)arr - 2;
		
		basePtr = realloc( basePtr, sizeof( size_t ) * 2 + maxLen * elementSize );
		basePtr[0] = maxLen;
		
		arr = basePtr;
	}
	
	return arr;
}

#define arr_push( arr, val ) ( (arr) = arr_alloc_head_internal( arr, sizeof( val ) ), *((typeof(val)*)arr_head_ptr_internal( arr, sizeof( val ) )) = (val) )
#define arr_extend( arr ) ( (arr) = arr_alloc_head_internal( arr, sizeof( *(arr) ) ), (typeof(arr))arr_head_ptr_internal( arr, sizeof( *(arr) ) ) )

static void arr_clear( void* arr ) {
	((size_t*)arr)[-1] = 0;
}

static void arr_free( void* arr ) {
	void* basePtr = (size_t*)arr - 2;
	free( basePtr );
}

#endif
