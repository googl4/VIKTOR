#include "types.h"

typedef struct {
	size_t blockSize;
	u32 numBlocks;
	u64* blocks;
	u64 miniBuffer[4];
} memoryPool_t;

void setupMemoryPool( memoryPool_t* pool, u32 numBlocks, size_t blockSize );
u32 reserveBlock( memoryPool_t* pool );
void releaseBlock( memoryPool_t* pool, u32 block );

typedef struct {
	memoryPool_t allocatorPool;
	memoryPool_t* chunks;
	size_t chunkSize;
	u32 numChunks;
	size_t minBlockSize;
} allocator_t;

#define NUM_CHUNKS_DEFAULT 64
#define MIN_BLOCK_SIZE_DEFAULT 4096

void setupAllocator( allocator_t* allocator, u32 numChunks, size_t chunkSize, size_t minBlockSize );
size_t alloc( allocator_t* allocator, size_t size, size_t alignment );
void dealloc( allocator_t* allocator, size_t address );
