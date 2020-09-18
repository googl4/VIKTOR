#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

extern "C" {
#include "viktor.h"
#include "matrix.h"

#include "testShaders.h"
}

int main( int argc, const char* argv[] ) {
	setvbuf( stdout, NULL, _IOLBF, 256 );
	
	VKTR_Init( "VIKTOR test" );
	VKTR_Queue queue = VKTR_GetQueue( VKTR_QUEUETYPE_GRAPHICS );
	VKTR_Window window = VKTR_CreateWindow( 1280, 720, "VIKTOR test", (VKTR_WindowFlags)( VKTR_WINDOWFLAG_VSYNC | VKTR_WINDOWFLAG_TRIPLEBUFFER ) );
	VKTR_CommandBuffer cmd = VKTR_CreateCommandBuffer( VKTR_QUEUETYPE_GRAPHICS );
	
	VKTR_Shader vertShader = VKTR_LoadShader( vertShaderData, vertShaderSize );
	VKTR_Shader fragShader = VKTR_LoadShader( fragShaderData, fragShaderSize );
	
	VKTR_Shader vertShader2 = VKTR_LoadShader( vertShaderData2, vertShaderSize2 );
	VKTR_Shader fragShader2 = VKTR_LoadShader( fragShaderData2, fragShaderSize2 );
	
	VKTR_Image depthBuf = VKTR_CreateImage( 1280, 720, VKTR_FORMAT_D32F, VKTR_IMAGEFLAG_RENDERTARGET );
	VKTR_Image renderBuf = VKTR_CreateImage( 1280, 720, VKTR_FORMAT_RGBA16F, (VKTR_ImageFlags)( VKTR_IMAGEFLAG_RENDERTARGET | VKTR_IMAGEFLAG_SUBPASSINPUT /*| VKTR_IMAGEFLAG_MULTISAMPLE_4X*/ ) );
	
	VKTR_Image swapchainImages[3];
	VKTR_RenderPass renderPasses[3];
	VKTR_Pipeline pipelines[3][2];
	for( int i = 0; i < 3; i++ ) {
		swapchainImages[i] = VKTR_GetSwapchainImage( window, i );
		
		renderPasses[i] = VKTR_CreateRenderPass( 3, 2 );
		VKTR_SetRenderPassAttachment( renderPasses[i], 0, swapchainImages[i], VKTR_FORMAT_RGBA8_SRGB, FALSE, TRUE );
		VKTR_SetRenderPassAttachment( renderPasses[i], 1, depthBuf, VKTR_FORMAT_D32F, FALSE, FALSE );
		VKTR_SetRenderPassAttachment( renderPasses[i], 2, renderBuf, VKTR_FORMAT_RGBA16F, FALSE, FALSE );
		
		VKTR_SetSubPassOutput( renderPasses[i], 0, 2, 0 );
		//VKTR_SetSubPassOutput( renderPasses[i], 0, 0, 0 );
		VKTR_SetSubPassDepthAttachment( renderPasses[i], 0, 1 );
		
		VKTR_SetSubPassInput( renderPasses[i], 1, 2, 0 );
		VKTR_SetSubPassOutput( renderPasses[i], 1, 0, 0 );
		
		VKTR_BuildRenderPass( renderPasses[i] );
		
		// https://gpuopen.com/learn/unlock-the-rasterizer-with-out-of-order-rasterization/
		
		pipelines[i][0] = VKTR_CreatePipeline( renderPasses[i], 0 );
		VKTR_PipelineSetShader( pipelines[i][0], VKTR_SHADERSTAGE_VERTEX, vertShader );
		VKTR_PipelineSetShader( pipelines[i][0], VKTR_SHADERSTAGE_FRAGMENT, fragShader );
		VKTR_PipelineSetDepthFunction( pipelines[i][0], VKTR_DEPTHFUNCTION_LT );
		VKTR_PipelineSetDepthEnable( pipelines[i][0], TRUE, TRUE );
		VKTR_PipelineSetViewport( pipelines[i][0], 0, 0, 0.0, 1280, 720, 1.0 );
		VKTR_PipelineSetScissor( pipelines[i][0], 0, 0, 1280, 720 );
		VKTR_PipelineSetBlendEnable( pipelines[i][0], 0, FALSE );
		VKTR_PipelineSetVertexBinding( pipelines[i][0], 0, 0, 12, VKTR_FORMAT_RGB32F );
		VKTR_PipelineSetVertexBinding( pipelines[i][0], 1, 0, 12, VKTR_FORMAT_RGB32F );
		VKTR_BuildPipeline( pipelines[i][0] );
		
		pipelines[i][1] = VKTR_CreatePipeline( renderPasses[i], 1 );
		VKTR_PipelineSetShader( pipelines[i][1], VKTR_SHADERSTAGE_VERTEX, vertShader2 );
		VKTR_PipelineSetShader( pipelines[i][1], VKTR_SHADERSTAGE_FRAGMENT, fragShader2 );
		VKTR_PipelineSetDepthEnable( pipelines[i][1], FALSE, FALSE );
		VKTR_PipelineSetViewport( pipelines[i][1], 0, 0, 0.0, 1280, 720, 1.0 );
		VKTR_PipelineSetScissor( pipelines[i][1], 0, 0, 1280, 720 );
		VKTR_PipelineSetBlendEnable( pipelines[i][1], 0, FALSE );
		//VKTR_PipelineSetVertexBinding( pipelines[i][1], 0, 0, 12, VKTR_FORMAT_RGB32F );
		//VKTR_PipelineSetVertexBinding( pipelines[i][1], 1, 0, 12, VKTR_FORMAT_RGB32F );
		VKTR_BuildPipeline( pipelines[i][1] );
	}
	
	VKTR_Semaphore imageAvailable = VKTR_CreateSemaphore();
	VKTR_Semaphore renderFinished = VKTR_CreateSemaphore();
	
	tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
	
	// https://github.com/zeux/meshoptimizer
	
	tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "carrier-proto.obj" );
	
	int numVerts = shapes[0].mesh.indices.size();
	
	VKTR_Buffer vtxBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_STAGING );
	VKTR_Buffer nrmBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_STAGING );
	VKTR_Buffer idxBuf = VKTR_CreateBuffer( numVerts * 4, VKTR_BUFFERTYPE_STAGING );
	
	vec3* vtxPtr = (vec3*)VKTR_MapBuffer( vtxBuf );
	vec3* nrmPtr = (vec3*)VKTR_MapBuffer( nrmBuf );
	u32* idxPtr = (u32*)VKTR_MapBuffer( idxBuf );
	
	for( int i = 0; i < numVerts; i++ ) {
		idxPtr[i] = i;
		
		int vi = shapes[0].mesh.indices[i].vertex_index;
		vtxPtr[i][0] = attrib.vertices[vi*3+0];
		vtxPtr[i][1] = attrib.vertices[vi*3+1];
		vtxPtr[i][2] = attrib.vertices[vi*3+2];
		
		int ni = shapes[0].mesh.indices[i].normal_index;
		nrmPtr[i][0] = attrib.normals[ni*3+0];
		nrmPtr[i][1] = attrib.normals[ni*3+1];
		nrmPtr[i][2] = attrib.normals[ni*3+2];
	}
	
	mat4 projection = mat4_perspective( 45.0f, 16.0f / 9.0f, 0.1f, 10.0f );
	projection = mat4_scale_aniso( projection, (vec4){ 1, -1, 1, 1 } );
	
	int frame = 0;
	
	fflush( stdout );
	
	while( !VKTR_WindowClosed( window ) ) {
		u32 img = VKTR_GetNextImage( window, imageAvailable );
		
		float a = (float)frame / 80.0f;
		mat4 view = mat4_lookat( (vec4){ cosf( a ) * 4.0f, 2.0f, sinf( a ) * 4.0f }, (vec4){ 0, 1, 0 }, (vec4){ 0, 1, 0 } );
		mat4 vp = mat4_mul( projection, view );
		
		vec4 lightDir = vec4_norm( (vec4){ 0.1, -1, 0.1 } );
		
		VKTR_StartRecording( cmd );
		VKTR_CMD_StartRenderPass( cmd, renderPasses[img] );
		VKTR_CMD_BindPipeline( cmd, pipelines[img][0] );
		
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_VERTEX, 0, 16 * 4, &vp );
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 64, 16, &lightDir );
		
		//VKTR_CMD_Clear( cmd, 0, (vec4){ 0, 0, 0, 1 } );
		VKTR_CMD_BindVertexBuffer( cmd, vtxBuf, 0 );
		VKTR_CMD_BindVertexBuffer( cmd, nrmBuf, 1 );
		VKTR_CMD_DrawIndexed( cmd, idxBuf, VKTR_INDEXTYPE_U32, numVerts );
		
		VKTR_CMD_NextSubPass( cmd );
		VKTR_CMD_BindPipeline( cmd, pipelines[img][1] );
		
		// bind descriptor set
		VKTR_CMD_Draw( cmd, 3 );
		
		VKTR_CMD_EndRenderPass( cmd );
		VKTR_EndRecording( cmd );
		VKTR_Submit( queue, cmd, imageAvailable, renderFinished );
		VKTR_Update( queue, window, renderFinished );
		
		frame++;
	}
	
	return 0;
}
