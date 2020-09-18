#ifndef _VIKTOR_H
#define _VIKTOR_H

#include <stddef.h>

#include "types.h"

typedef enum {
	VKTR_QUEUETYPE_GRAPHICS,
	VKTR_QUEUETYPE_COMPUTE,
	VKTR_QUEUETYPE_TRANSFER
} VKTR_QueueType;

typedef enum {
	VKTR_BUFFERTYPE_DEVICE_GENERIC,
	VKTR_BUFFERTYPE_DEVICE_VERTEX,
	VKTR_BUFFERTYPE_DEVICE_INDEX,
	VKTR_BUFFERTYPE_DEVICE_UNIFORM,
	VKTR_BUFFERTYPE_HOST,
	VKTR_BUFFERTYPE_STAGING
} VKTR_BufferType;

typedef enum {
	VKTR_SHADERSTAGE_VERTEX,
	VKTR_SHADERSTAGE_FRAGMENT,
	VKTR_SHADERSTAGE_COMPUTE,
	VKTR_SHADERSTAGE_GEOMETRY
} VKTR_ShaderStage;

typedef enum {
	VKTR_FORMAT_RGBA8,
	VKTR_FORMAT_RGBA8_SRGB,
	VKTR_FORMAT_RGBA16F,
	VKTR_FORMAT_RGBA32F,
	VKTR_FORMAT_RGB32F,
	VKTR_FORMAT_D32F
} VKTR_Format;

typedef enum {
	VKTR_DEPTHFUNCTION_LT,
	VKTR_DEPTHFUNCTION_LTE,
	VKTR_DEPTHFUNCTION_EQ,
	VKTR_DEPTHFUNCTION_NEQ,
	VKTR_DEPTHFUNCTION_GTE,
	VKTR_DEPTHFUNCTION_GT,
	VKTR_DEPTHFUNCTION_ALWAYS,
	VKTR_DEPTHFUNCTION_NEVER
} VKTR_DepthFunction;

typedef enum {
	VKTR_INDEXTYPE_U16,
	VKTR_INDEXTYPE_U32
} VKTR_IndexType;

typedef enum {
	VKTR_BLENDOP_ADD,
	VKTR_BLENDOP_SUB,
	VKTR_BLENDOP_SUB_REVERSE,
	VKTR_BLENDOP_MIN,
	VKTR_BLENDOP_MAX
} VKTR_BlendOp;

typedef enum {
	VKTR_BLEND_ZERO,
	VKTR_BLEND_ONE,
	VKTR_BLEND_SRC_ALPHA,
	VKTR_BLEND_DST_ALPHA,
	VKTR_BLEND_SRC_COLOUR,
	VKTR_BLEND_DST_COLOUR,
	VKTR_BLEND_SRC_ALPHA_INVERSE,
	VKTR_BLEND_DST_ALPHA_INVERSE,
	VKTR_BLEND_SRC_COLOUR_INVERSE,
	VKTR_BLEND_DST_COLOUR_INVERSE
} VKTR_BlendFactor;

typedef enum {
	VKTR_IMAGEFLAG_LINEARDATA = 0x01,
	VKTR_IMAGEFLAG_RENDERTARGET = 0x02,
	VKTR_IMAGEFLAG_MULTISAMPLE_2X = 0x04,
	VKTR_IMAGEFLAG_MULTISAMPLE_4X = 0x08,
	VKTR_IMAGEFLAG_MULTISAMPLE_8X = 0x10,
	VKTR_IMAGEFLAG_SUBPASSINPUT = 0x20
} VKTR_ImageFlags;

typedef enum {
	VKTR_WINDOWFLAG_VSYNC = 0x01,
	VKTR_WINDOWFLAG_TRIPLEBUFFER = 0x02
} VKTR_WindowFlags;

typedef enum {
	VKTR_DESCRIPTOR_SAMPLER,
	VKTR_DESCRIPTOR_COMBINED_SAMPLER,
	VKTR_DESCRIPTOR_IMAGE,
	VKTR_DESCRIPTOR_UNIFORM,
	VKTR_DESCRIPTOR_INPUT_ATTACHMENT,
	VKTR_DESCRIPTOR_INLINE_UNIFORM
} VKTR_DescriptorType;

typedef struct {
	int x, y, w, h;
} VKTR_CopyRegion;

typedef void* VKTR_Window;
typedef void* VKTR_Queue;
typedef void* VKTR_CommandBuffer;
typedef void* VKTR_Buffer;
typedef void* VKTR_Image;
typedef void* VKTR_Shader;
typedef void* VKTR_Pipeline;
typedef void* VKTR_RenderPass;
typedef void* VKTR_Semaphore;

const static void* VKTR_INVALID_HANDLE = NULL;

void VKTR_Init( const char* appName );
VKTR_Queue VKTR_GetQueue( VKTR_QueueType type );

VKTR_Window VKTR_CreateWindow( u32 width, u32 height, const char* title, VKTR_WindowFlags flags );
int VKTR_WindowClosed( VKTR_Window window );
void VKTR_Update( VKTR_Queue queue, VKTR_Window window, VKTR_Semaphore waitSemaphore );
//VKTR_Image VKTR_GetWindowSurface( VKTR_Window window );
VKTR_Image VKTR_GetSwapchainImage( VKTR_Window window, u32 n );
u32 VKTR_GetNextImage( VKTR_Window window, VKTR_Semaphore availableSemaphore );

VKTR_Semaphore VKTR_CreateSemaphore( void );
void VKTR_FreeSemaphore( VKTR_Semaphore semaphore );

VKTR_Buffer VKTR_CreateBuffer( u64 size, VKTR_BufferType type );
void* VKTR_MapBuffer( VKTR_Buffer buffer );
void VKTR_FlushBuffer( VKTR_Buffer buffer, u64 offset, u64 size );
void VKTR_FreeBuffer( VKTR_Buffer buffer );

//VKTR_Image VKTR_CreateImage( VKTR_Buffer buffer, u32 width, u32 height, VKTR_Format type, VKTR_ImageFlags flags, VKTR_BufferType bufferType );
//VKTR_Buffer VKTR_GetImageBuffer( VKTR_Image image );
//void VKTR_FreeImage( VKTR_Image image );
VKTR_Image VKTR_CreateImage( u32 width, u32 height, VKTR_Format type, VKTR_ImageFlags flags );

VKTR_Shader VKTR_LoadShader( const void* data, u64 dataSize );
void VKTR_FreeShader( VKTR_Shader shader );

VKTR_RenderPass VKTR_CreateRenderPass( u32 numAttachments, u32 numSubPasses );
void VKTR_SetRenderPassAttachment( VKTR_RenderPass renderPass, u32 attachment, VKTR_Image image, VKTR_Format format, int loadInput, int storeOutput );
void VKTR_SetSubPassInput( VKTR_RenderPass renderPass, u32 subPass, u32 attachment, u32 binding );
void VKTR_SetSubPassOutput( VKTR_RenderPass renderPass, u32 subPass, u32 attachment, u32 binding );
void VKTR_SetSubPassDepthAttachment( VKTR_RenderPass renderPass, u32 subPass, u32 attachment );
void VKTR_BuildRenderPass( VKTR_RenderPass renderPass );
void VKTR_FreeRenderPass( VKTR_RenderPass renderPass );

VKTR_Pipeline VKTR_CreatePipeline( VKTR_RenderPass renderPass, u32 subPass );
//void VKTR_PipelineSetRenderTargets( VKTR_Pipeline pipeline, VKTR_Image* images, u32 numTargets );
void VKTR_PipelineSetShader( VKTR_Pipeline pipeline, VKTR_ShaderStage stage, VKTR_Shader shader );
void VKTR_PipelineSetDepthFunction( VKTR_Pipeline pipeline, VKTR_DepthFunction function );
void VKTR_PipelineSetDepthEnable( VKTR_Pipeline pipeline, int test, int write );
void VKTR_PipelineSetViewport( VKTR_Pipeline pipeline, float x, float y, float z, float width, float height, float depth );
void VKTR_PipelineSetScissor( VKTR_Pipeline pipeline, u32 x, u32 y, u32 width, u32 height );
void VKTR_PipelineSetBlend( VKTR_Pipeline pipeline, VKTR_BlendOp blendOp, VKTR_BlendFactor src, VKTR_BlendFactor dst );
void VKTR_PipelineSetAlphaBlend( VKTR_Pipeline pipeline, VKTR_BlendOp blendOp, VKTR_BlendFactor src, VKTR_BlendFactor dst );
void VKTR_PipelineSetBlendEnable( VKTR_Pipeline pipeline, u32 attachment, int enable );
void VKTR_PipelineSetVertexBinding( VKTR_Pipeline pipeline, u32 binding, u32 offset, u32 stride, VKTR_Format format );
void VKTR_BuildPipeline( VKTR_Pipeline pipeline );
void VKTR_FreePipeline( VKTR_Pipeline pipeline );

VKTR_CommandBuffer VKTR_CreateCommandBuffer( VKTR_QueueType type );
void VKTR_StartRecording( VKTR_CommandBuffer buffer );
void VKTR_EndRecording( VKTR_CommandBuffer buffer );
void VKTR_FreeCommandBuffer( VKTR_CommandBuffer buffer );

void VKTR_Submit( VKTR_Queue queue, VKTR_CommandBuffer buffer, VKTR_Semaphore waitSemaphore, VKTR_Semaphore signalSemaphore );

void VKTR_CMD_BindVertexBuffer( VKTR_CommandBuffer buffer, VKTR_Buffer vertexBuffer, int binding );
void VKTR_CMD_DrawIndexed( VKTR_CommandBuffer buffer, VKTR_Buffer indexBuffer, VKTR_IndexType indexType, u32 count );
void VKTR_CMD_Draw( VKTR_CommandBuffer buffer, u32 count );
void VKTR_CMD_Copy( VKTR_CommandBuffer buffer, VKTR_Buffer src, u64 srcOffset, VKTR_Buffer dst, u64 dstOffset, u64 size );
void VKTR_CMD_Blit( VKTR_CommandBuffer buffer, VKTR_Image src, VKTR_CopyRegion copyRegion, VKTR_Image dst, VKTR_CopyRegion dstRegion );
void VKTR_CMD_Clear( VKTR_CommandBuffer buffer, u32 attachment, vec4 clearVal );
void VKTR_CMD_ClearDepth( VKTR_CommandBuffer buffer, float clearVal );
void VKTR_CMD_PushConst( VKTR_CommandBuffer buffer, VKTR_ShaderStage stage, u32 offset, u32 size, void* data );
void VKTR_CMD_UpdateBuffer( VKTR_CommandBuffer buffer, u32 offset, u32 size, void* data );
void VKTR_CMD_Dispatch( VKTR_CommandBuffer buffer, u32 x, u32 y, u32 z );
void VKTR_CMD_BindPipeline( VKTR_CommandBuffer buffer, VKTR_Pipeline pipeline );
void VKTR_CMD_StartRenderPass( VKTR_CommandBuffer buffer, VKTR_RenderPass renderPass );
void VKTR_CMD_NextSubPass( VKTR_CommandBuffer buffer );
void VKTR_CMD_EndRenderPass( VKTR_CommandBuffer buffer );

#endif
