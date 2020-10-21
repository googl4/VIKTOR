#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "alloc.h"

void setupMemoryPool( memoryPool_t* pool, u32 numBlocks, size_t blockSize ) {
	pool->blockSize = blockSize;
	pool->numBlocks = numBlocks;
	size_t bitmapSize = ( numBlocks + 63 ) / 64;
	if( bitmapSize <= 4 ) {
		pool->blocks = pool->miniBuffer;
		memset( pool->miniBuffer, 0, sizeof( pool->miniBuffer ) );
	} else {
		pool->blocks = calloc( bitmapSize, sizeof( u64 ) );
	}
}

u32 reserveBlock( memoryPool_t* pool ) {
	size_t bitmapSize = ( pool->numBlocks + 63 ) / 64;
	
	for( int i = 0; i < bitmapSize; i++ ) {
		int n = __builtin_ffsll( ~pool->blocks[i] );
		if( n > 0 ) {
			n -= 1;
			if( __builtin_expect( i * 64 + n < pool->numBlocks, TRUE ) ) {
				pool->blocks[i] |= 1 << n;
				return i * 64 + n;
			} else {
				return UINT32_MAX;
			}
		}
	}
	
	return UINT32_MAX;
}

void releaseBlock( memoryPool_t* pool, u32 block ) {
	pool->blocks[block / 64] &= ~( 1 << ( block % 64 ) );
}



void setupAllocator( allocator_t* allocator, u32 numChunks, size_t chunkSize, size_t minBlockSize ) {
	setupMemoryPool( &allocator->allocatorPool, numChunks, chunkSize );
	allocator->chunks = calloc( numChunks, sizeof( memoryPool_t ) );
	allocator->numChunks = numChunks;
	allocator->chunkSize = chunkSize;
}

u64 pow2( u64 n ) {
	return 1ull << ( 64 - __builtin_clzll( n - 1 ) );
}

size_t alloc( allocator_t* allocator, size_t size, size_t alignment ) {
	if( __builtin_expect( alignment > size, FALSE ) ) {
		size = alignment;
	}
	
	size_t blockSize = pow2( size );
	
	if( size < allocator->minBlockSize ) {
		blockSize = allocator->minBlockSize;
	}
	
	if( __builtin_expect( blockSize > allocator->chunkSize, FALSE ) ) {
		return SIZE_MAX;
	}
	
	for( int i = 0; i < allocator->numChunks; i++ ) {
		if( allocator->chunks[i].blockSize == blockSize ) {
			u32 b = reserveBlock( &allocator->chunks[i] );
			
			if( b != UINT32_MAX ) {
				return i * allocator->chunkSize + b * allocator->chunks[i].blockSize;
			}
		}
	}
	
	u32 nc = reserveBlock( &allocator->allocatorPool );
	
	if( nc == UINT32_MAX ) {
		return SIZE_MAX;
	}
	
	setupMemoryPool( &allocator->chunks[nc], allocator->chunkSize / blockSize, blockSize );
	u32 b = reserveBlock( &allocator->chunks[nc] );
	
	return nc * allocator->chunkSize + b * allocator->chunks[nc].blockSize;
}

void dealloc( allocator_t* allocator, size_t address ) {
	u32 c = address / allocator->chunkSize;
	u32 b = ( address % allocator->chunkSize ) / allocator->chunks[c].blockSize;
	releaseBlock( &allocator->chunks[c], b );
}
