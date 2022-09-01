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


/*
typedef struct {
	u32 size;
	u16 next;
	u8 free;
} compactAllocatorBlock_t;

typedef struct {
	compactAllocatorBlock_t* blocks;
	u16 maxBlocks;
} compactAllocator_t;
*/

void setupCompactAllocator( compactAllocator_t* allocator, size_t size, u16 maxBlocks ) {
	allocator->maxBlocks = maxBlocks;
	allocator->blocks = calloc( maxBlocks, sizeof( compactAllocatorBlock_t ) );
	
	allocator->blocks[0].size = size;
	allocator->blocks[0].next = 1;
	allocator->blocks[0].free = TRUE;
}

size_t compactAlloc( compactAllocator_t* allocator, size_t size, size_t alignment ) {
	int minBlock = -1;
	u32 minBlockSize = UINT32_MAX;
	size_t minOffset = 0;
	u32 minAlign = 0;
	
	int searchBlock = 0;
	size_t offset = 0;
	
	while( allocator->blocks[searchBlock].size > 0 ) {
		u32 align = ( alignment - offset % alignment ) % alignment;
		u32 alignedSize = size + align;
		
		if( alignedSize <= allocator->blocks[searchBlock].size && allocator->blocks[searchBlock].size < minBlockSize && allocator->blocks[searchBlock].free ) {
			minBlock = searchBlock;
			minBlockSize = allocator->blocks[searchBlock].size;
			minOffset = offset;
			minAlign = align;
		}
		
		offset += allocator->blocks[searchBlock].size;
		searchBlock = allocator->blocks[searchBlock].next;
	}
	
	if( minBlock < 0 ) {
		return SIZE_MAX;
	}
	
	allocator->blocks[minBlock].free = FALSE;
	
	u32 alignedSize = size + minAlign;
	int nextBlock = allocator->blocks[minBlock].next;
	
	if( allocator->blocks[nextBlock].size == 0 ) {
		allocator->blocks[nextBlock].size = allocator->blocks[minBlock].size - alignedSize;
		allocator->blocks[nextBlock].free = TRUE;
		
		if( allocator->blocks[nextBlock+1].size == 0 ) { // FIX: nextBlock + 1 == maxBlocks
			allocator->blocks[nextBlock].next = nextBlock + 1;
			
		} else {
			for( int i = 0; i < allocator->maxBlocks; i++ ) {
				if( allocator->blocks[i].size == 0 ) {
					allocator->blocks[nextBlock].next = i;
					break;
				}
			}
		}
		
	} else if( allocator->blocks[nextBlock].free ) {
		allocator->blocks[nextBlock].size += allocator->blocks[minBlock].size - alignedSize;
		
	} else {
		int newBlock = -1;
		
		for( int i = 0; i < allocator->maxBlocks; i++ ) {
			if( allocator->blocks[i].size == 0 ) {
				newBlock = i;
				break;
			}
		}
		
		// FIX: newBlock < 0
		
		allocator->blocks[newBlock].size = allocator->blocks[minBlock].size - alignedSize;
		allocator->blocks[newBlock].next = nextBlock;
		allocator->blocks[newBlock].free = TRUE;
		
		allocator->blocks[minBlock].next = newBlock;
	}
	
	allocator->blocks[minBlock].size = alignedSize;
	
	return minOffset + minAlign;
}

void compactDealloc( compactAllocator_t* allocator, size_t offset ) {
	int searchBlock = 0;
	size_t searchOffset = 0;
	int prevBlock = -1;
	
	while( allocator->blocks[searchBlock].size > 0 ) {
		if( searchOffset + allocator->blocks[searchBlock].size > offset ) {
			if( prevBlock >= 0 ) {
				if( allocator->blocks[prevBlock].free ) {
					allocator->blocks[prevBlock].size += allocator->blocks[searchBlock].size;
					allocator->blocks[searchBlock].size = 0;
					allocator->blocks[prevBlock].next = allocator->blocks[searchBlock].next;
					
					searchBlock = prevBlock;
					//return;
				}
			}
			
			allocator->blocks[searchBlock].free = TRUE;
			
			int nextBlock = allocator->blocks[searchBlock].next;
			if( allocator->blocks[nextBlock].size > 0 && allocator->blocks[nextBlock].free ) {
				allocator->blocks[searchBlock].size += allocator->blocks[nextBlock].size;
				allocator->blocks[nextBlock].size = 0;
				allocator->blocks[searchBlock].next = allocator->blocks[nextBlock].next;
			}
			
			return;
		}
		
		searchOffset += allocator->blocks[searchBlock].size;
		prevBlock = searchBlock;
		searchBlock = allocator->blocks[searchBlock].next;
	}
}
