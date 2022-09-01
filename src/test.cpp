#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <cfloat>

//#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"
#include "avrt.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "ini.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern "C" {
#include "viktor.h"
#include "matrix.h"
#include "alloc.h"
#include "bvh.h"

#include "testShaders.h"
}

const size_t MAX_TEXTURES = 4096;

u32 rndState = 0x12345678;

u32 rnd( void ) {
	u32 x = rndState;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	rndState = x;
	return x;
}

float frnd( void ) {
	u32 n = rnd();
	u32 i = 0x3F800000 | ( n >> 9 );
	float f = *((float*)&i);
	f -= 1.0f;
	return f;
}

typedef struct {
	struct {
		int displayWidth;
		int displayHeight;
		int renderWidth;
		int renderHeight;
		bool vsync;
		bool tripleBuffer;
		bool enablePreDelay;
		float fpsLimit;
		float preDelayMargin;
		int timerPeriod;
		bool useMMCSS;
		bool borderless;
		bool checkerboard;
		bool depthResolveCB;
	} display;
} engineConfig_t;

int iniHandler( void* user, const char* section, const char* name, const char* value ) {
	engineConfig_t* cfg = (engineConfig_t*)user;
	
	if( strcasecmp( section, "Display" ) == 0 ) {
		if( strcasecmp( name, "iDisplayWidth" ) == 0 ) {
			cfg->display.displayWidth = atoi( value );
			
		} else if( strcasecmp( name, "iDisplayHeight" ) == 0 ) {
			cfg->display.displayHeight = atoi( value );
			
		} else if( strcasecmp( name, "iRenderWidth" ) == 0 ) {
			cfg->display.renderWidth = atoi( value );
			
		} else if( strcasecmp( name, "iRenderHeight" ) == 0 ) {
			cfg->display.renderHeight = atoi( value );
			
		} else if( strcasecmp( name, "bEnableVsync" ) == 0 ) {
			cfg->display.vsync = ( strcasecmp( value, "True" ) == 0 );
			
		} else if( strcasecmp( name, "bEnableTripleBuffer" ) == 0 ) {
			cfg->display.tripleBuffer = ( strcasecmp( value, "True" ) == 0 );
			
		} else if( strcasecmp( name, "bEnablePreDelay" ) == 0 ) {
			cfg->display.enablePreDelay = ( strcasecmp( value, "True" ) == 0 );
			
		} else if( strcasecmp( name, "fFPSLimit" ) == 0 ) {
			cfg->display.fpsLimit = atof( value );
			
		} else if( strcasecmp( name, "fPreDelayMarginMS" ) == 0 ) {
			cfg->display.preDelayMargin = atof( value );
			
		} else if( strcasecmp( name, "iOverrideTimerPeriod" ) == 0 ) {
			cfg->display.timerPeriod = atoi( value );
			
		} else if( strcasecmp( name, "bUseMMCSS" ) == 0 ) {
			cfg->display.useMMCSS = ( strcasecmp( value, "True" ) == 0 );
			
		} else if( strcasecmp( name, "bBorderless" ) == 0 ) {
			cfg->display.borderless = ( strcasecmp( value, "True" ) == 0 );
			
		} else if( strcasecmp( name, "bCheckerboard" ) == 0 ) {
			cfg->display.checkerboard = ( strcasecmp( value, "True" ) == 0 );
			
		} else if( strcasecmp( name, "bCBDepthResolve" ) == 0 ) {
			cfg->display.depthResolveCB = ( strcasecmp( value, "True" ) == 0 );
		}
	}
	
	return TRUE;
}

void setDefaultEngineConfig( engineConfig_t* config ) {
	config->display.displayWidth = 1280;
	config->display.displayHeight = 720;
	config->display.renderWidth = 1280;
	config->display.renderHeight = 720;
	config->display.vsync = TRUE;
	config->display.tripleBuffer = TRUE;
	config->display.enablePreDelay = FALSE;
	config->display.fpsLimit = 120.0f;
	config->display.preDelayMargin = 2.0f;
	config->display.timerPeriod = -1;
	config->display.useMMCSS = TRUE;
	config->display.borderless = FALSE;
	config->display.checkerboard = FALSE;
	config->display.depthResolveCB = FALSE;
}

void readEngineConfig( engineConfig_t* config, const char* iniData ) {
	setDefaultEngineConfig( config );
	
	int res = ini_parse_string( iniData, iniHandler, config );
	
	if( res < 0 ) {
		printf( "failed to read engine config file\n" );
	}
}

int main( int argc, const char* argv[] ) {
	setvbuf( stdout, NULL, _IOLBF, 256 );
	setvbuf( stderr, NULL, _IOLBF, 256 );
	
	rndState = time( NULL );
	
	
	//
	/*
	compactAllocator_t testAllocator;
	setupCompactAllocator( &testAllocator, 65536, 4096 );
	
	size_t a[256];
	//bool allocated[256] = {};
	
	for( int i = 0; i < 16; i++ ) {
		int sz = rnd() % 4096;
		a[i] = compactAlloc( &testAllocator, sz, 16 );
		//allocated[i] = true;
		printf( "alloc %d : %d\n", a[i], sz );
	}
	
	for( int i = 0; i < 16; i++ ) {
		if( rnd() % 2 == 0 ) {
			compactDealloc( &testAllocator, a[i] );
			printf( "free %d\n", a[i] );
		}
	}
	
	for( int i = 0; i < 16; i++ ) {
		int sz = rnd() % 4096;
		a[i] = compactAlloc( &testAllocator, sz, 16 );
		//allocated[i] = true;
		printf( "alloc %d : %d\n", a[i], sz );
	}
	
	
	return 0;
	*/
	//
	
	
	FILE* iniFile = fopen( "engine.ini", "r" );
	fseek( iniFile, 0, SEEK_END );
	size_t iniFileLen = ftell( iniFile );
	rewind( iniFile );
	char* iniData = (char*)malloc( iniFileLen + 1 );
	fread( iniData, iniFileLen, 1, iniFile );
	iniData[iniFileLen] = 0;
	fclose( iniFile );
	
	engineConfig_t config;
	readEngineConfig( &config, iniData );
	
	
	VKTR_Init( "VIKTOR test" );
	VKTR_Queue queue = VKTR_GetQueue( VKTR_QUEUETYPE_GRAPHICS );
	//VKTR_Queue computeQueue = VKTR_GetQueue( VKTR_QUEUETYPE_COMPUTE );
	VKTR_Queue transferQueue = VKTR_GetQueue( VKTR_QUEUETYPE_TRANSFER );
	
	u32 windowFlags = 0;
	if( config.display.vsync ) {
		windowFlags |= VKTR_WINDOWFLAG_VSYNC;
	}
	if( config.display.tripleBuffer ) {
		windowFlags |= VKTR_WINDOWFLAG_TRIPLEBUFFER;
	}
	if( config.display.borderless ) {
		windowFlags |= VKTR_WINDOWFLAG_BORDERLESS;
	}
	
	VKTR_Window window = VKTR_CreateWindow( config.display.displayWidth, config.display.displayHeight, "VIKTOR test", (VKTR_WindowFlags)windowFlags );
	
	VKTR_CommandPool cmdPool = VKTR_CreateCommandPool( VKTR_QUEUETYPE_GRAPHICS );
	//VKTR_CommandPool computeCmdPool = VKTR_CreateCommandPool( VKTR_QUEUETYPE_COMPUTE );
	VKTR_CommandPool transferCmdPool = VKTR_CreateCommandPool( VKTR_QUEUETYPE_TRANSFER );
	
	//VKTR_CommandBuffer cmd = VKTR_CreateCommandBuffer( cmdPool );
	
	VKTR_Shader vertShader = VKTR_LoadShader( vertShaderData, vertShaderSize );
	VKTR_Shader fragShader = VKTR_LoadShader( fragShaderData, fragShaderSize );
	
	VKTR_Shader vertShader2 = VKTR_LoadShader( vertShaderData2, vertShaderSize2 );
	VKTR_Shader fragShader2 = VKTR_LoadShader( fragShaderData2, fragShaderSize2 );
	
	VKTR_Shader CBResolveShader = VKTR_LoadShader( CBResolveShaderData, CBResolveShaderSize );
	
	VKTR_Shader CBLightCullShader = VKTR_LoadShader( CBLightCullShaderData, CBLightCullShaderSize );
	VKTR_Shader lightCullShader = VKTR_LoadShader( lightCullShaderData, lightCullShaderSize );
	
	int renderSamples;
	
	VKTR_Image depthBuf, renderBuf;
	if( config.display.checkerboard ) {
		depthBuf = VKTR_CreateImage( config.display.renderWidth / 2, config.display.renderHeight / 2, VKTR_FORMAT_D32F, (VKTR_ImageFlags)( VKTR_IMAGEFLAG_RENDERTARGET | VKTR_IMAGEFLAG_MULTISAMPLE_2X ), 1 );
		renderBuf = VKTR_CreateImage( config.display.renderWidth / 2, config.display.renderHeight / 2, VKTR_FORMAT_RGBA16F, (VKTR_ImageFlags)( VKTR_IMAGEFLAG_RENDERTARGET | VKTR_IMAGEFLAG_MULTISAMPLE_2X ), 1 );
		renderSamples = 2;
		
	} else {
		depthBuf = VKTR_CreateImage( config.display.renderWidth, config.display.renderHeight, VKTR_FORMAT_D32F, VKTR_IMAGEFLAG_RENDERTARGET, 1 );
		renderBuf = VKTR_CreateImage( config.display.renderWidth, config.display.renderHeight, VKTR_FORMAT_RGBA16F, VKTR_IMAGEFLAG_RENDERTARGET, 1 );
		renderSamples = 1;
	}
	
	VKTR_Sampler sampler = VKTR_CreateSampler( VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_WRAP_REPEAT, 0, FALSE );
	VKTR_Sampler postSampler = VKTR_CreateSampler( VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_WRAP_CLAMP, 0, FALSE );
	
	VKTR_Sampler shadowSampler = VKTR_CreateSampler( VKTR_FILTER_NEAREST, VKTR_FILTER_NEAREST, VKTR_FILTER_NEAREST, VKTR_WRAP_BORDER, 0, FALSE );//TRUE );
	
	int numSwapchainImages = VKTR_GetSwapchainLength( window );
	
	VKTR_RenderPass preZRenderPass = VKTR_CreateRenderPass( 1, 1 );
	VKTR_SetRenderPassAttachment( preZRenderPass, 0, VKTR_FORMAT_D32F, renderSamples, FALSE, TRUE, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT );
	VKTR_SetSubPassDepthAttachment( preZRenderPass, 0, 0 );
	VKTR_BuildRenderPass( preZRenderPass );
	
	VKTR_Pipeline prePassPipeline = VKTR_CreatePipeline( preZRenderPass, 0, 3, 0 );
	VKTR_PipelineSetShader( prePassPipeline, VKTR_SHADERSTAGE_VERTEX, vertShader );
	VKTR_PipelineSetDepthFunction( prePassPipeline, VKTR_DEPTHFUNCTION_GT );
	VKTR_PipelineSetDepthEnable( prePassPipeline, TRUE, TRUE );
	VKTR_PipelineSetVertexBinding( prePassPipeline, 0, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( prePassPipeline, 1, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( prePassPipeline, 2, 0, 8, VKTR_FORMAT_RG32F );
	VKTR_PipelineSetPushConstRange( prePassPipeline, 0, 64, VKTR_SHADERSTAGE_VERTEX );
	VKTR_BuildPipeline( prePassPipeline );
	
	VKTR_FrameBuffer prePassFrameBuffer = VKTR_CreateFrameBuffer( preZRenderPass );
	VKTR_SetFrameBufferAttachment( prePassFrameBuffer, 0, depthBuf );
	VKTR_BuildFrameBuffer( prePassFrameBuffer );
	
	VKTR_RenderPass mainRenderPass = VKTR_CreateRenderPass( 2, 1 );
	VKTR_SetRenderPassAttachment( mainRenderPass, 0, VKTR_FORMAT_RGBA16F, renderSamples, FALSE, TRUE, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_SHADER_READ );
	VKTR_SetRenderPassAttachment( mainRenderPass, 1, VKTR_FORMAT_D32F, renderSamples, TRUE, FALSE, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT, VKTR_IMAGELAYOUT_SHADER_READ );
	
	VKTR_SetSubPassOutput( mainRenderPass, 0, 0, 0 );
	VKTR_SetSubPassDepthAttachment( mainRenderPass, 0, 1 );
	
	VKTR_BuildRenderPass( mainRenderPass );
	
	VKTR_DescriptorPool descriptorPool = VKTR_CreateDescriptorPool( MAX_TEXTURES, 8, 0 );
	
	VKTR_Pipeline mainPipeline = VKTR_CreatePipeline( mainRenderPass, 0, 3, 1 );
	VKTR_PipelineSetShader( mainPipeline, VKTR_SHADERSTAGE_VERTEX, vertShader );
	VKTR_PipelineSetShader( mainPipeline, VKTR_SHADERSTAGE_FRAGMENT, fragShader );
	VKTR_PipelineSetDepthFunction( mainPipeline, VKTR_DEPTHFUNCTION_GTE );
	VKTR_PipelineSetDepthEnable( mainPipeline, TRUE, FALSE ); // TODO alpha test prepass
	VKTR_PipelineSetBlend( mainPipeline, 0, VKTR_BLENDOP_ADD, VKTR_BLEND_SRC_ALPHA, VKTR_BLEND_SRC_ALPHA_INVERSE );
	VKTR_PipelineSetBlendEnable( mainPipeline, 0, TRUE );
	VKTR_PipelineSetVertexBinding( mainPipeline, 0, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( mainPipeline, 1, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( mainPipeline, 2, 0, 8, VKTR_FORMAT_RG32F );
	VKTR_PipelineSetPushConstRange( mainPipeline, 0, 64, VKTR_SHADERSTAGE_VERTEX );
	VKTR_PipelineSetPushConstRange( mainPipeline, 64, 60, VKTR_SHADERSTAGE_FRAGMENT );
	VKTR_PipelineSetDescriptorBinding( mainPipeline, 0, 0, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, MAX_TEXTURES );
	VKTR_PipelineSetImmutableSamplerBinding( mainPipeline, 0, 1, VKTR_SHADERSTAGE_FRAGMENT, &sampler );
	VKTR_PipelineSetDescriptorBinding( mainPipeline, 0, 2, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 1 );
	VKTR_PipelineSetImmutableSamplerBinding( mainPipeline, 0, 3, VKTR_SHADERSTAGE_FRAGMENT, &shadowSampler );
	VKTR_PipelineSetDescriptorBinding( mainPipeline, 0, 4, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_UNIFORM, 1 );
	VKTR_PipelineSetDescriptorBinding( mainPipeline, 0, 5, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_BUFFER, 1 );
	VKTR_PipelineSetDescriptorBinding( mainPipeline, 0, 6, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_BUFFER, 1 );
	VKTR_PipelineSetDescriptorBinding( mainPipeline, 0, 7, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_BUFFER, 1 );
	VKTR_BuildPipeline( mainPipeline );
	
	VKTR_DescriptorSet mainDescriptorSet = VKTR_CreateDescriptorSet( descriptorPool, mainPipeline, 0 );
	
	VKTR_FrameBuffer mainFrameBuffer = VKTR_CreateFrameBuffer( mainRenderPass );
	VKTR_SetFrameBufferAttachment( mainFrameBuffer, 0, renderBuf );
	VKTR_SetFrameBufferAttachment( mainFrameBuffer, 1, depthBuf );
	VKTR_BuildFrameBuffer( mainFrameBuffer );
	
	VKTR_Image swapchainImages[3];
	for( int i = 0; i < numSwapchainImages; i++ ) {
		swapchainImages[i] = VKTR_GetSwapchainImage( window, i );
	}
	
	VKTR_RenderPass postRenderPass = VKTR_CreateRenderPass( 1, 1 );
	VKTR_SetRenderPassAttachment( postRenderPass, 0, VKTR_FORMAT_RGBA8_SRGB, 1, FALSE, TRUE, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_PRESENT );
	VKTR_SetSubPassOutput( postRenderPass, 0, 0, 0 );
	VKTR_BuildRenderPass( postRenderPass );
	
	VKTR_Pipeline postPipeline = VKTR_CreatePipeline( postRenderPass, 0, 0, 1 );
	VKTR_PipelineSetShader( postPipeline, VKTR_SHADERSTAGE_VERTEX, vertShader2 );
	
	if( config.display.checkerboard ) {
		VKTR_PipelineSetShader( postPipeline, VKTR_SHADERSTAGE_FRAGMENT, CBResolveShader );
		
	} else {
		VKTR_PipelineSetShader( postPipeline, VKTR_SHADERSTAGE_FRAGMENT, fragShader2 );
	}
	
	VKTR_PipelineSetDepthEnable( postPipeline, FALSE, FALSE );
	VKTR_PipelineSetBlendEnable( postPipeline, 0, FALSE );
	VKTR_PipelineSetDescriptorBinding( postPipeline, 0, 0, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 1 );
	VKTR_PipelineSetDescriptorBinding( postPipeline, 0, 1, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 1 );
	VKTR_PipelineSetImmutableSamplerBinding( postPipeline, 0, 2, VKTR_SHADERSTAGE_FRAGMENT, &postSampler );
	VKTR_BuildPipeline( postPipeline );
		
	VKTR_DescriptorSet postDescriptorSet = VKTR_CreateDescriptorSet( descriptorPool, postPipeline, 0 );
	VKTR_SetTextureBinding( postDescriptorSet, 0, 0, renderBuf );
	VKTR_SetTextureBinding( postDescriptorSet, 1, 0, depthBuf );
	
	VKTR_FrameBuffer postFrameBuffers[3];
	for( int i = 0; i < numSwapchainImages; i++ ) {
		postFrameBuffers[i] = VKTR_CreateFrameBuffer( postRenderPass );
		VKTR_SetFrameBufferAttachment( postFrameBuffers[i], 0, swapchainImages[i] );
		VKTR_BuildFrameBuffer( postFrameBuffers[i] );
	}
	/*
	VKTR_Image swapchainImages[3];
	VKTR_RenderPass renderPasses[3];
	VKTR_Pipeline pipelines[3];
	VKTR_DescriptorSet descriptorSets[3];
	for( int i = 0; i < numSwapchainImages; i++ ) {
		swapchainImages[i] = VKTR_GetSwapchainImage( window, i );
		
		renderPasses[i] = VKTR_CreateRenderPass( 1, 1 );
		VKTR_SetRenderPassAttachment( renderPasses[i], 0, swapchainImages[i], FALSE, TRUE, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_PRESENT );
		VKTR_SetSubPassOutput( renderPasses[i], 0, 0, 0 );
		VKTR_BuildRenderPass( renderPasses[i] );
		
		pipelines[i] = VKTR_CreatePipeline( renderPasses[i], 0, 0, 1 );
		VKTR_PipelineSetShader( pipelines[i], VKTR_SHADERSTAGE_VERTEX, vertShader2 );
		
		if( config.display.checkerboard ) {
			VKTR_PipelineSetShader( pipelines[i], VKTR_SHADERSTAGE_FRAGMENT, CBResolveShader );
			
		} else {
			VKTR_PipelineSetShader( pipelines[i], VKTR_SHADERSTAGE_FRAGMENT, fragShader2 );
		}
		
		VKTR_PipelineSetDepthEnable( pipelines[i], FALSE, FALSE );
		VKTR_PipelineSetBlendEnable( pipelines[i], 0, FALSE );
		VKTR_PipelineSetDescriptorBinding( pipelines[i], 0, 0, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 1 );
		VKTR_PipelineSetDescriptorBinding( pipelines[i], 0, 1, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 1 );
		VKTR_PipelineSetImmutableSamplerBinding( pipelines[i], 0, 2, VKTR_SHADERSTAGE_FRAGMENT, &sampler );
		VKTR_BuildPipeline( pipelines[i] );
		
		descriptorSets[i] = VKTR_CreateDescriptorSet( descriptorPool, pipelines[i], 0 );
		VKTR_SetTextureBinding( descriptorSets[i], 0, 0, renderBuf );
		VKTR_SetTextureBinding( descriptorSets[i], 1, 0, depthBuf );
	}
	*/
	
	VKTR_CommandPool frameCmdPools[8][2];
	VKTR_CommandPool frameCmdBufs[8][4];
	VKTR_Fence frameCmdFences[8];
	for( int i = 0; i < 8; i++ ) {
		frameCmdPools[i][0] = VKTR_CreateCommandPool( VKTR_QUEUETYPE_GRAPHICS );
		//frameCmdPools[i][1] = VKTR_CreateCommandPool( VKTR_QUEUETYPE_COMPUTE );
		frameCmdPools[i][1] = VKTR_CreateCommandPool( VKTR_QUEUETYPE_GRAPHICS );
		frameCmdBufs[i][0] = VKTR_CreateCommandBuffer( frameCmdPools[i][0] );
		frameCmdBufs[i][1] = VKTR_CreateCommandBuffer( frameCmdPools[i][0] );
		frameCmdBufs[i][2] = VKTR_CreateCommandBuffer( frameCmdPools[i][1] );
		frameCmdBufs[i][3] = VKTR_CreateCommandBuffer( frameCmdPools[i][0] );
		frameCmdFences[i] = VKTR_CreateFence( TRUE );
	}
	
	VKTR_Semaphore imageAvailable = VKTR_CreateSemaphore();
	VKTR_Semaphore renderFinished = VKTR_CreateSemaphore();
	
	tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
	
	// https://github.com/zeux/meshoptimizer
	
	tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "assets/crytek-sponza/sponza-tri.obj", "assets/crytek-sponza/" );
	//tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "assets/sponza2/sponza.obj", "assets/sponza2/" );
	//tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "assets/SanMiguel/san-miguel-low-poly.obj", "assets/SanMiguel/" );
	//tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "assets/FA38/FA38_landed.obj", "assets/FA38/" );
	//tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "assets/fireplaceRoom/fireplace_room.obj", "assets/fireplaceRoom/" );
	
	/*
	printf( "%d materials\n", materials.size() );
	for( int i = 0; i < materials.size(); i++ ) {
		printf( "  %d: %s\n", i, materials[i].diffuse_texname.c_str() );
	}
	return 0;
	*/
	
	VKTR_Image textures[MAX_TEXTURES];
	//VKTR_CommandBuffer uploadCmd = VKTR_CreateCommandBuffer( VKTR_QUEUETYPE_TRANSFER );
	VKTR_CommandBuffer uploadCmd = VKTR_CreateCommandBuffer( transferCmdPool );
	VKTR_StartRecording( uploadCmd );
	
	u32 numTextures = 0;
	u8 transparentMaterials[MAX_TEXTURES];
	std::string textureNames[MAX_TEXTURES];
	
	u16* materialMap = (u16*)malloc( materials.size() * 2 );
	
	for( int i = 0; i < materials.size(); i++ ) {
		if( materials[i].diffuse_texname.length() > 0 ) {
			int existingTexture = -1;
			for( int j = 0; j < numTextures; j++ ) {
				if( textureNames[j] == materials[i].diffuse_texname ) {
					existingTexture = j;
					break;
				}
			}
			
			if( existingTexture >= 0 ) {
				materialMap[i] = existingTexture;
				
			} else {
				std::string pathStr = "assets/crytek-sponza/" + materials[i].diffuse_texname;
				//std::string pathStr = "assets/sponza2/" + materials[i].diffuse_texname;
				//std::string pathStr = "assets/SanMiguel/" + materials[i].diffuse_texname;
				//std::string pathStr = "assets/FA38/" + materials[i].diffuse_texname;
				//std::string pathStr = "assets/fireplaceRoom/" + materials[i].diffuse_texname;
				
				const char* texname = pathStr.c_str();
				printf( "mat %d: %s ", numTextures, texname );
				int w, h, c;
				u8* imgData = stbi_load( texname, &w, &h, &c, 4 );
				printf( "%dx%d ", w, h );
				
				textures[numTextures] = VKTR_CreateImage( w, h, VKTR_FORMAT_RGBA8_SRGB, (VKTR_ImageFlags)0, 1 );
				transparentMaterials[numTextures] = ( c == 4 ) || ( c == 2 );
				
				printf( "%d\n", c );
				
				VKTR_SetTextureBinding( mainDescriptorSet, 0, numTextures, textures[numTextures] );
				
				VKTR_Buffer uploadBuffer = VKTR_CreateBuffer( w * h * 4, VKTR_BUFFERTYPE_STAGING );
				void* uploadPtr = VKTR_MapBuffer( uploadBuffer );
				memcpy( uploadPtr, imgData, w * h * 4 );
				
				VKTR_CMD_TransferImageOwnership( uploadCmd, textures[numTextures], VKTR_QUEUETYPE_TRANSFER, VKTR_QUEUETYPE_TRANSFER, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_TRANSFER_DST );
				
				VKTR_CMD_CopyToImage( uploadCmd, uploadBuffer, textures[numTextures] );
				
				VKTR_CMD_TransferImageOwnership( uploadCmd, textures[numTextures], VKTR_QUEUETYPE_TRANSFER, VKTR_QUEUETYPE_GRAPHICS, VKTR_IMAGELAYOUT_TRANSFER_DST, VKTR_IMAGELAYOUT_SHADER_READ );
				
				textureNames[numTextures] = materials[numTextures].diffuse_texname;
				materialMap[i] = numTextures;
				
				numTextures++;
			}
			
		} else {
			materialMap[i] = 0;
		}
	}
	
	VKTR_EndRecording( uploadCmd );
	
	VKTR_Semaphore uploadSemaphore = VKTR_CreateSemaphore();
	
	//VKTR_Submit( queue, uploadCmd, VKTR_INVALID_HANDLE, VKTR_INVALID_HANDLE );
	VKTR_Submit( transferQueue, uploadCmd, 0, NULL, 1, &uploadSemaphore, VKTR_INVALID_HANDLE );
	
	VKTR_CommandBuffer cmd = VKTR_CreateCommandBuffer( cmdPool );
	VKTR_StartRecording( cmd );
	for( int i = 0; i < numTextures; i++ ) {
		VKTR_CMD_TransferImageOwnership( cmd, textures[i], VKTR_QUEUETYPE_TRANSFER, VKTR_QUEUETYPE_GRAPHICS, VKTR_IMAGELAYOUT_TRANSFER_DST, VKTR_IMAGELAYOUT_SHADER_READ );
	}
	VKTR_EndRecording( cmd );
	VKTR_Submit( queue, cmd, 1, &uploadSemaphore, 0, NULL, VKTR_INVALID_HANDLE );
	
	int numVerts = shapes[0].mesh.indices.size();
	
	mesh_t originalMesh;
	originalMesh.idx = (u32*)malloc( numVerts * sizeof( u32 ) );
	originalMesh.vtx = (vec3*)malloc( numVerts * sizeof( vec3 ) );
	originalMesh.mat = (u16*)malloc( ( numVerts / 3 ) * sizeof( u16 ) );
	originalMesh.numVerts = numVerts;
	originalMesh.numFaces = numVerts / 3;
	
	vec3* meshNormals = (vec3*)malloc( numVerts * sizeof( vec3 ) );
	vec2* meshUVs = (vec2*)malloc( numVerts * sizeof( vec2 ) );
	
	for( int i = 0; i < numVerts; i++ ) {
		originalMesh.idx[i] = i;
		
		int vi = shapes[0].mesh.indices[i].vertex_index;
		originalMesh.vtx[i][0] = attrib.vertices[vi*3+0];
		originalMesh.vtx[i][1] = attrib.vertices[vi*3+1];
		originalMesh.vtx[i][2] = attrib.vertices[vi*3+2];
		
		int ni = shapes[0].mesh.indices[i].normal_index;
		meshNormals[i][0] = attrib.normals[ni*3+0];
		meshNormals[i][1] = attrib.normals[ni*3+1];
		meshNormals[i][2] = attrib.normals[ni*3+2];
		
		int ti = shapes[0].mesh.indices[i].texcoord_index;
		meshUVs[i][0] = attrib.texcoords[ti*2+0];
		meshUVs[i][1] = 1.0f - attrib.texcoords[ti*2+1];
		
		int material = materialMap[shapes[0].mesh.material_ids[i/3]];
		originalMesh.mat[i/3] = material;
	}
	
	mesh_t newMesh = buildChunkedMesh( originalMesh, 256, CHUNK_RADIAL | CHUNK_NORMAL );
	
	VKTR_Buffer vtxBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer nrmBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer uvBuf = VKTR_CreateBuffer( numVerts * 8, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer idxBuf = VKTR_CreateBuffer( numVerts * 4, VKTR_BUFFERTYPE_DEVICE_INDEX );
	
	vec3* vtxPtr = (vec3*)VKTR_MapBuffer( vtxBuf );
	vec3* nrmPtr = (vec3*)VKTR_MapBuffer( nrmBuf );
	vec2* uvPtr = (vec2*)VKTR_MapBuffer( uvBuf );
	u32* idxPtr = (u32*)VKTR_MapBuffer( idxBuf );
	
	memcpy( vtxPtr, newMesh.vtx, numVerts * sizeof( vec3 ) );
	memcpy( nrmPtr, meshNormals, numVerts * sizeof( vec3 ) );
	memcpy( uvPtr, meshUVs, numVerts * sizeof( vec2 ) );
	memcpy( idxPtr, newMesh.idx, numVerts * sizeof( u32 ) );
	
	u32 drawBatches[16384][3] = {};
	int numDrawBatches = 0;
	int currentMaterial = newMesh.mat[0];
	
	for( int i = 0; i < numVerts / 3; i++ ) {
		int material = newMesh.mat[i];
		
		if( material != currentMaterial ) {
			drawBatches[numDrawBatches][1] = i - drawBatches[numDrawBatches][0];
			drawBatches[numDrawBatches][2] = currentMaterial;
			numDrawBatches++;
			drawBatches[numDrawBatches][0] = i;
			currentMaterial = material;
			
			printf( "draw batch %llu: %llu %llu\n", numDrawBatches - 1, drawBatches[numDrawBatches-1][2], drawBatches[numDrawBatches-1][1] );
			//fflush( stdout );
		}
	}
	
	/*
	VKTR_Buffer vtxBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer nrmBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer uvBuf = VKTR_CreateBuffer( numVerts * 8, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer idxBuf = VKTR_CreateBuffer( numVerts * 4, VKTR_BUFFERTYPE_DEVICE_INDEX );
	
	vec3* vtxPtr = (vec3*)VKTR_MapBuffer( vtxBuf );
	vec3* nrmPtr = (vec3*)VKTR_MapBuffer( nrmBuf );
	vec2* uvPtr = (vec2*)VKTR_MapBuffer( uvBuf );
	u32* idxPtr = (u32*)VKTR_MapBuffer( idxBuf );
	
	u32 drawBatches[4096][3] = {};
	int numDrawBatches = 0;
	int currentMaterial = materialMap[shapes[0].mesh.material_ids[0]];
	
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
		
		int ti = shapes[0].mesh.indices[i].texcoord_index;
		uvPtr[i][0] = attrib.texcoords[ti*2+0];
		uvPtr[i][1] = 1.0f - attrib.texcoords[ti*2+1];
		
		int material = materialMap[shapes[0].mesh.material_ids[i/3]];
		if( material != currentMaterial ) {
			drawBatches[numDrawBatches][1] = i / 3 - drawBatches[numDrawBatches][0];
			drawBatches[numDrawBatches][2] = currentMaterial;
			numDrawBatches++;
			drawBatches[numDrawBatches][0] = i / 3;
			currentMaterial = material;
			
			printf( "draw batch %llu: %llu %llu\n", numDrawBatches - 1, drawBatches[numDrawBatches-1][2], drawBatches[numDrawBatches-1][1] );
			//fflush( stdout );
		}
	}
	*/
	
	mat4 projection = mat4_perspectiveReverseZ( 35.0f / 57.2958f, (float)config.display.displayWidth / config.display.displayHeight, 0.1f );
	projection = mat4_scale_aniso( projection, (vec4){ 1, -1, 1, 1 } );
	
	vec4 camPos = (vec4){ -1200.0f, 100.0f, 0.0f };

	/*
	u32 numLights = 256;
	vec4 lights[256];
	
	for( int i = 0; i < numLights; i++ ) {
		lights[i][0] = frnd() * 1000;
		lights[i][1] = frnd() * 20;
		lights[i][2] = frnd() * 300 - 150;
		lights[i][3] = frnd() * 200;
	}
	
	VKTR_Buffer lightBuf = VKTR_CreateBuffer( sizeof( lights ), VKTR_BUFFERTYPE_DEVICE_UNIFORM );
	void* lightPtr = VKTR_MapBuffer( lightBuf );
	memcpy( lightPtr, lights, sizeof( lights ) );
	VKTR_FlushBuffer( lightBuf, 0, sizeof( lights ) );
	
	VKTR_SetUniformBinding( mainDescriptorSet, 2, 0, lightBuf, 0, sizeof( lights ) );
	*/
	
	VKTR_Buffer lightBuf = VKTR_CreateBuffer( 1024 * 16, VKTR_BUFFERTYPE_DEVICE_GENERIC );
	VKTR_Buffer lightColBuf = VKTR_CreateBuffer( 1024 * 16, VKTR_BUFFERTYPE_DEVICE_GENERIC );
	vec4* lightPtr = (vec4*)VKTR_MapBuffer( lightBuf );
	vec4* lightColPtr = (vec4*)VKTR_MapBuffer( lightColBuf );
	
	u32 numLights = 30;
	
	for( int i = 0; i < numLights; i++ ) {
		lightPtr[i][0] = frnd() * 2000 - 1000;
		lightPtr[i][1] = frnd() * 500;
		lightPtr[i][2] = frnd() * 300 - 150;
		//lightPtr[i][3] = 20;//frnd() * 10 + 5;
		
		lightColPtr[i][0] = frnd() * 2500 + 2000;
		lightColPtr[i][1] = frnd() * 2500 + 2000;
		lightColPtr[i][2] = frnd() * 2500 + 2000;
		
		float lightColMax = fmaxf( fmaxf( lightColPtr[i][0], lightColPtr[i][1] ), lightColPtr[i][2] );
		
		// l = ( 1.0 / ( 1.0 + d * d ) ) * i
		
		float lightThreshold = 0.01f;
		lightPtr[i][3] = sqrtf( 1.0f / ( lightThreshold / lightColMax ) - 1.0f );
	}
	
	VKTR_FlushBuffer( lightBuf, 0, 1024 * 16 );
	VKTR_FlushBuffer( lightColBuf, 0, 1024 * 16 );
	
	VKTR_SetBufferBinding( mainDescriptorSet, 5, 0, lightBuf, 0, 1024 * 16 );
	VKTR_SetBufferBinding( mainDescriptorSet, 6, 0, lightColBuf, 0, 1024 * 16 );
	
	const int lightCullTileSize = 8;
	const int tileMaxLights = 30;
	size_t lightCullTilesX = ( config.display.renderWidth + lightCullTileSize - 1 ) / lightCullTileSize;
	size_t lightCullTilesY = ( config.display.renderHeight + lightCullTileSize - 1 ) / lightCullTileSize;
	
	struct {
		float camPos[3];
		u32 numLights;
		float forward[3];
		float ar;
		float right[3];
		float scale;
		float up[3];
	} lightCullUniformBuf;
	
	lightCullUniformBuf.camPos[0] = camPos[0];
	lightCullUniformBuf.camPos[1] = camPos[1];
	lightCullUniformBuf.camPos[2] = camPos[2];
	lightCullUniformBuf.numLights = numLights;
	lightCullUniformBuf.forward[0] = 1;
	lightCullUniformBuf.forward[1] = 0;
	lightCullUniformBuf.forward[2] = 0;
	lightCullUniformBuf.ar = (float)config.display.displayWidth / config.display.displayHeight;
	lightCullUniformBuf.right[0] = 0;
	lightCullUniformBuf.right[1] = 0;
	lightCullUniformBuf.right[2] = 1;
	lightCullUniformBuf.scale = tanf( ( 35.0f / 57.2958f ) / 2.0f );
	lightCullUniformBuf.up[0] = 0;
	lightCullUniformBuf.up[1] = 1;
	lightCullUniformBuf.up[2] = 0;
	
	size_t tileLightsBufSize = lightCullTilesY * lightCullTilesX * ( tileMaxLights + 1 ) * 4;
	VKTR_Buffer tileLightsBuf = VKTR_CreateBuffer( tileLightsBufSize, VKTR_BUFFERTYPE_DEVICE_GENERIC );
	
	VKTR_SetBufferBinding( mainDescriptorSet, 7, 0, tileLightsBuf, 0, tileLightsBufSize );
	
	VKTR_Pipeline lightCullPipeline = VKTR_CreateComputePipeline( 1 );
	if( config.display.checkerboard ) {
		VKTR_PipelineSetShader( lightCullPipeline, VKTR_SHADERSTAGE_COMPUTE, CBLightCullShader );
	} else {
		VKTR_PipelineSetShader( lightCullPipeline, VKTR_SHADERSTAGE_COMPUTE, lightCullShader );
	}
	VKTR_PipelineSetPushConstRange( lightCullPipeline, 0, 60, VKTR_SHADERSTAGE_COMPUTE );
	VKTR_PipelineSetDescriptorBinding( lightCullPipeline, 0, 0, VKTR_SHADERSTAGE_COMPUTE, VKTR_DESCRIPTOR_IMAGE, 1 );
	VKTR_PipelineSetImmutableSamplerBinding( lightCullPipeline, 0, 1, VKTR_SHADERSTAGE_COMPUTE, &sampler );
	VKTR_PipelineSetDescriptorBinding( lightCullPipeline, 0, 2, VKTR_SHADERSTAGE_COMPUTE, VKTR_DESCRIPTOR_BUFFER, 1 );
	VKTR_PipelineSetDescriptorBinding( lightCullPipeline, 0, 3, VKTR_SHADERSTAGE_COMPUTE, VKTR_DESCRIPTOR_BUFFER, 1 );
	VKTR_BuildPipeline( lightCullPipeline );
	
	VKTR_DescriptorSet lightCullDescriptorSet = VKTR_CreateDescriptorSet( descriptorPool, lightCullPipeline, 0 );
	VKTR_SetTextureBinding( lightCullDescriptorSet, 0, 0, depthBuf );
	VKTR_SetBufferBinding( lightCullDescriptorSet, 2, 0, tileLightsBuf, 0, tileLightsBufSize );
	VKTR_SetBufferBinding( lightCullDescriptorSet, 3, 0, lightBuf, 0, 1024 * 16 );
	
	//VKTR_CommandBuffer lightCullCmd = VKTR_CreateCommandBuffer( cmdPool );
	
	VKTR_Image shadowTex = VKTR_CreateImage( 4096, 4096, VKTR_FORMAT_D32F, VKTR_IMAGEFLAG_RENDERTARGET, 2 );
	VKTR_Image shadowTexView1 = VKTR_CreateImageView( shadowTex, VKTR_FORMAT_D32F, 0 );
	VKTR_Image shadowTexView2 = VKTR_CreateImageView( shadowTex, VKTR_FORMAT_D32F, 1 );
	
	VKTR_RenderPass shadowPass = VKTR_CreateRenderPass( 1, 1 );
	VKTR_SetRenderPassAttachment( shadowPass, 0, VKTR_FORMAT_D32F, 1, FALSE, TRUE, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_SHADER_READ );
	VKTR_SetSubPassDepthAttachment( shadowPass, 0, 0 );
	VKTR_BuildRenderPass( shadowPass );
	
	VKTR_Pipeline shadowPipeline = VKTR_CreatePipeline( shadowPass, 0, 3, 0 );
	VKTR_PipelineSetShader( shadowPipeline, VKTR_SHADERSTAGE_VERTEX, vertShader );
	VKTR_PipelineSetDepthFunction( shadowPipeline, VKTR_DEPTHFUNCTION_LT );
	VKTR_PipelineSetDepthEnable( shadowPipeline, TRUE, TRUE );
	VKTR_PipelineSetDepthClamp( shadowPipeline, TRUE );
	VKTR_PipelineSetDepthBias( shadowPipeline, TRUE, 0.0f, 2.0f, 0.0f );
	VKTR_PipelineSetVertexBinding( shadowPipeline, 0, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( shadowPipeline, 1, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( shadowPipeline, 2, 0, 8, VKTR_FORMAT_RG32F );
	VKTR_PipelineSetPushConstRange( shadowPipeline, 0, 64, VKTR_SHADERSTAGE_VERTEX );
	VKTR_BuildPipeline( shadowPipeline );
	
	VKTR_FrameBuffer shadowFrameBuffer1 = VKTR_CreateFrameBuffer( shadowPass );
	VKTR_SetFrameBufferAttachment( shadowFrameBuffer1, 0, shadowTexView1 );
	VKTR_BuildFrameBuffer( shadowFrameBuffer1 );
	
	VKTR_FrameBuffer shadowFrameBuffer2 = VKTR_CreateFrameBuffer( shadowPass );
	VKTR_SetFrameBufferAttachment( shadowFrameBuffer2, 0, shadowTexView2 );
	VKTR_BuildFrameBuffer( shadowFrameBuffer2 );
	
	VKTR_SetTextureBinding( mainDescriptorSet, 2, 0, shadowTex );
	
	VKTR_Semaphore prePassSemaphore = VKTR_CreateSemaphore();
	VKTR_Semaphore prePassSemaphore2 = VKTR_CreateSemaphore();
	VKTR_Semaphore lightCullSemaphore = VKTR_CreateSemaphore();
	VKTR_Semaphore shadowSemaphore = VKTR_CreateSemaphore();
	
	VKTR_Buffer shadowMatBuf = VKTR_CreateBuffer( 64 * 2, VKTR_BUFFERTYPE_DEVICE_UNIFORM );
	mat4* shadowMatPtr = (mat4*)VKTR_MapBuffer( shadowMatBuf );
	VKTR_SetUniformBinding( mainDescriptorSet, 4, 0, shadowMatBuf, 0, 64 * 2 );
	
	u64 frame = 0;
	
	fflush( stdout );
	
	DWORD MMCSS_TaskIndex;
	HANDLE MMCSS_TaskHandle;
	if( config.display.useMMCSS ) {
		AvSetMmThreadCharacteristics( "Games", &MMCSS_TaskIndex );
	}
	
	if( config.display.timerPeriod > 0 ) {
		timeBeginPeriod( config.display.timerPeriod );
	}
	
	s64 counterFreq;
	s64 frameStart, framePreDelay, frameAcquire, frameSubmit, frameLimiterDelay, frameEnd;
	QueryPerformanceFrequency( (LARGE_INTEGER*)&counterFreq );
	int frameTimeSlackMS = 0;
	float preDelayMarginMS = config.display.preDelayMargin;
	float frameProcessTimeVarianceMS = 0.0f;
	float lastFrameProcessTimeMS = 0.0f;
	float frameTimeLimitMS = 1000.0f / config.display.fpsLimit;
	
	QueryPerformanceCounter( (LARGE_INTEGER*)&frameLimiterDelay );
	
	while( !VKTR_WindowClosed( window ) ) {
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameStart );
		
		//printf( "PREDELAY\n" );
		//fflush( stdout );
		
		if( frameTimeSlackMS > 0 && config.display.enablePreDelay ) {
			Sleep( frameTimeSlackMS );
		}
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&framePreDelay );
		
		//printf( "ACQUIRE\n" );
		//fflush( stdout );
		
		u32 img = VKTR_GetNextImage( window, imageAvailable );
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameAcquire );
		
		mat4 view = mat4_lookat( camPos, (vec4){ 1, 100, 0 }, (vec4){ 0, 1, 0 } );
		mat4 vp = mat4_mul( projection, view );
		
		//printf( "CMDRESET\n" );
		//fflush( stdout );
		
		int frameCmdIndex = frame % 8;
		
		VKTR_WaitFence( frameCmdFences[frameCmdIndex], 1000000000 );
		VKTR_ResetFence( frameCmdFences[frameCmdIndex] );
		
		VKTR_ResetCommandPool( frameCmdPools[frameCmdIndex][0] );
		VKTR_ResetCommandPool( frameCmdPools[frameCmdIndex][1] );
		
		VKTR_CommandBuffer cmd = frameCmdBufs[frameCmdIndex][0];
		
		//printf( "PREZSTART\n" );
		//fflush( stdout );
		
		VKTR_StartRecording( cmd );
		VKTR_CMD_StartRenderPass( cmd, preZRenderPass, prePassFrameBuffer );
		VKTR_CMD_BindPipeline( cmd, prePassPipeline );
		
		VKTR_CMD_BindVertexBuffer( cmd, vtxBuf, 0 );
		VKTR_CMD_BindVertexBuffer( cmd, nrmBuf, 1 );
		VKTR_CMD_BindVertexBuffer( cmd, uvBuf, 2 );
		
		VKTR_CMD_BindIndexBuffer( cmd, idxBuf, VKTR_INDEXTYPE_U32 );
		
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_VERTEX, 0, 16 * 4, &vp );
		
		// opaque pre-pass
		for( int i = 0; i < numDrawBatches; i++ ) {
			u32 tIdx = drawBatches[i][2];
			//if( tIdx == 2 ) {
				//tIdx = 0;
			//}
			if( !transparentMaterials[tIdx] ) {
				//VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 80, 4, &tIdx );
				VKTR_CMD_DrawIndexedOffset( cmd, drawBatches[i][0] * 3, drawBatches[i][1] * 3, 0 );
			}
		}
		
		VKTR_CMD_EndRenderPass( cmd );
		
		//VKTR_CMD_TransferBufferOwnership( cmd, tileLightsBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_COMPUTE );
		//VKTR_CMD_TransferImageOwnership( cmd, depthBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_COMPUTE, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT, VKTR_IMAGELAYOUT_SHADER_READ );
		//VKTR_CMD_TransferBufferOwnership( cmd, lightBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_COMPUTE );
		VKTR_CMD_TransferImageOwnership( cmd, depthBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_GRAPHICS, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT, VKTR_IMAGELAYOUT_SHADER_READ );
		
		VKTR_EndRecording( cmd );
		
		//printf( "PREZEND\n" );
		//fflush( stdout );
		
		VKTR_Semaphore prePassSemaphores[2] = { prePassSemaphore, prePassSemaphore2 };
		VKTR_Submit( queue, cmd, 1, &imageAvailable, 2, prePassSemaphores, VKTR_INVALID_HANDLE );
		
		//printf( "CULLSTART\n" );
		//fflush( stdout );
		
		VKTR_CommandBuffer lightCullCmd = frameCmdBufs[frameCmdIndex][2];
		VKTR_StartRecording( lightCullCmd );
		
		//VKTR_CMD_TransferBufferOwnership( lightCullCmd, tileLightsBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_COMPUTE );
		//VKTR_CMD_TransferImageOwnership( lightCullCmd, depthBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_COMPUTE, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT, VKTR_IMAGELAYOUT_SHADER_READ );
		//VKTR_CMD_TransferBufferOwnership( lightCullCmd, lightBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_COMPUTE );
		
		VKTR_CMD_BindPipeline( lightCullCmd, lightCullPipeline );
		VKTR_CMD_BindDescriptorSet( lightCullCmd, lightCullDescriptorSet );
		VKTR_CMD_PushConst( lightCullCmd, VKTR_SHADERSTAGE_COMPUTE, 0, 60, &lightCullUniformBuf );
		VKTR_CMD_Dispatch( lightCullCmd, lightCullTilesX, lightCullTilesY, 1 );
		
		//VKTR_CMD_TransferBufferOwnership( lightCullCmd, tileLightsBuf, VKTR_QUEUETYPE_COMPUTE, VKTR_QUEUETYPE_GRAPHICS );
		//VKTR_CMD_TransferImageOwnership( lightCullCmd, depthBuf, VKTR_QUEUETYPE_COMPUTE, VKTR_QUEUETYPE_GRAPHICS, VKTR_IMAGELAYOUT_SHADER_READ, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT );
		//VKTR_CMD_TransferBufferOwnership( lightCullCmd, lightBuf, VKTR_QUEUETYPE_COMPUTE, VKTR_QUEUETYPE_GRAPHICS );
		VKTR_CMD_TransferImageOwnership( lightCullCmd, depthBuf, VKTR_QUEUETYPE_GRAPHICS, VKTR_QUEUETYPE_GRAPHICS, VKTR_IMAGELAYOUT_SHADER_READ, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT );
		
		VKTR_EndRecording( lightCullCmd );
		
		//printf( "CULLEND\n" );
		//fflush( stdout );
		
		//VKTR_Submit( computeQueue, lightCullCmd, 1, &prePassSemaphore2, 1, &lightCullSemaphore, VKTR_INVALID_HANDLE );
		VKTR_Submit( queue, lightCullCmd, 1, &prePassSemaphore2, 1, &lightCullSemaphore, VKTR_INVALID_HANDLE );
		
		//SHADOWSTART
		
		vec4 sunDir = (vec4){ cos( (float)frame / 1000.0f ) * -0.2f, -1.0f, sin( (float)frame / 1000.0f ) * -0.1f };
		float shadowDist1 = 500.0f;
		float shadowDist2 = 1500.0f;
		
		mat4 shadowView = mat4_lookat( (vec4){ 0, 0, 0 }, sunDir, (vec4){ 0, 1, 0 } );
		
		mat4 invVP = mat4_invert( vp );
		mat4 NDCtoShadow = mat4_mul( shadowView, invVP );
		
		mat4 shadowProjection[2];
		mat4 shadowMat[2];
		
		for( int i = 0; i < 2; i++ ) {
			float d1, d2;
			
			if( i == 0 ) {
				d1 = 0.1f;
				d2 = shadowDist1;
				
			} else {
				d1 = shadowDist1;
				d2 = shadowDist2;
			}
			
			float nearZ = 0.1f / d1;
			float farZ = 0.1f / d2;
			
			vec4 viewCornersNDC[8] = {
				{ -1, -1, nearZ, 1 },
				{  1, -1, nearZ, 1 },
				{ -1,  1, nearZ, 1 },
				{  1,  1, nearZ, 1 },
				{ -1, -1, farZ, 1 },
				{  1, -1, farZ, 1 },
				{ -1,  1, farZ, 1 },
				{  1,  1, farZ, 1 }
			};
			
			vec4 vcMin = (vec4){ FLT_MAX, FLT_MAX, FLT_MAX };
			vec4 vcMax = (vec4){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
			
			for( int i = 0; i < 8; i++ ) {
				vec4 viewCorner = mat4_mul_vec4( NDCtoShadow, viewCornersNDC[i] );
				
				viewCorner /= viewCorner[3];
				
				vcMin = vec4_min( vcMin, viewCorner );
				vcMax = vec4_max( vcMax, viewCorner );
				
				//printf( "frustum corner %d: %f, %f, %f, %f\n", i, viewCorner[0], viewCorner[1], viewCorner[2], viewCorner[3] );
			}
			
			shadowProjection[i] = mat4_ortho( vcMin[0], vcMax[0], vcMin[1], vcMax[1], vcMin[2], vcMax[2] );
			
			shadowMat[i] = mat4_mul( shadowProjection[i], shadowView );
			shadowMat[i] = mat4_scale_aniso( shadowMat[i], (vec4){ 1, -1, 1, 1 } );
		}
		
		shadowMatPtr[0] = shadowMat[0];
		shadowMatPtr[1] = shadowMat[1];
		
		VKTR_CommandBuffer shadowCmd = frameCmdBufs[frameCmdIndex][3];
		VKTR_StartRecording( shadowCmd );
		
		// cascade 1
		VKTR_CMD_StartRenderPass( shadowCmd, shadowPass, shadowFrameBuffer1 );
		
		VKTR_CMD_ClearDepth( shadowCmd, 1.0f, (vec4){ 0, 0, 4096, 4096 } );
		
		VKTR_CMD_BindPipeline( shadowCmd, shadowPipeline );
		
		VKTR_CMD_BindVertexBuffer( shadowCmd, vtxBuf, 0 );
		VKTR_CMD_BindVertexBuffer( shadowCmd, nrmBuf, 1 );
		VKTR_CMD_BindVertexBuffer( shadowCmd, uvBuf, 2 );
		
		VKTR_CMD_BindIndexBuffer( shadowCmd, idxBuf, VKTR_INDEXTYPE_U32 );
		
		VKTR_CMD_PushConst( shadowCmd, VKTR_SHADERSTAGE_VERTEX, 0, 16 * 4, &shadowMat[0] );
		
		// opaque shadow pass
		for( int i = 0; i < numDrawBatches; i++ ) {
			u32 tIdx = drawBatches[i][2];
			if( !transparentMaterials[tIdx] ) {
				//VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 80, 4, &tIdx );
				VKTR_CMD_DrawIndexedOffset( shadowCmd, drawBatches[i][0] * 3, drawBatches[i][1] * 3, 0 );
			}
		}
		
		VKTR_CMD_EndRenderPass( shadowCmd );
		
		// cascade 2
		VKTR_CMD_StartRenderPass( shadowCmd, shadowPass, shadowFrameBuffer2 );
		
		VKTR_CMD_ClearDepth( shadowCmd, 1.0f, (vec4){ 0, 0, 4096, 4096 } );
		
		VKTR_CMD_BindPipeline( shadowCmd, shadowPipeline );
		
		VKTR_CMD_BindVertexBuffer( shadowCmd, vtxBuf, 0 );
		VKTR_CMD_BindVertexBuffer( shadowCmd, nrmBuf, 1 );
		VKTR_CMD_BindVertexBuffer( shadowCmd, uvBuf, 2 );
		
		VKTR_CMD_BindIndexBuffer( shadowCmd, idxBuf, VKTR_INDEXTYPE_U32 );
		
		VKTR_CMD_PushConst( shadowCmd, VKTR_SHADERSTAGE_VERTEX, 0, 16 * 4, &shadowMat[1] );
		
		// opaque shadow pass
		for( int i = 0; i < numDrawBatches; i++ ) {
			u32 tIdx = drawBatches[i][2];
			if( !transparentMaterials[tIdx] ) {
				//VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 80, 4, &tIdx );
				VKTR_CMD_DrawIndexedOffset( shadowCmd, drawBatches[i][0] * 3, drawBatches[i][1] * 3, 0 );
			}
		}
		
		VKTR_CMD_EndRenderPass( shadowCmd );
		
		VKTR_EndRecording( shadowCmd );
		
		//SHADOWEND
		
		VKTR_Submit( queue, shadowCmd, 1, &prePassSemaphore, 1, &shadowSemaphore, VKTR_INVALID_HANDLE );
		
		//printf( "MAINSTART\n" );
		//fflush( stdout );
		
		cmd = frameCmdBufs[frameCmdIndex][1];
		VKTR_StartRecording( cmd );
		
		//VKTR_CMD_TransferBufferOwnership( cmd, tileLightsBuf, VKTR_QUEUETYPE_COMPUTE, VKTR_QUEUETYPE_GRAPHICS );
		//VKTR_CMD_TransferImageOwnership( cmd, depthBuf, VKTR_QUEUETYPE_COMPUTE, VKTR_QUEUETYPE_GRAPHICS, VKTR_IMAGELAYOUT_SHADER_READ, VKTR_IMAGELAYOUT_DEPTH_ATTACHMENT );
		//VKTR_CMD_TransferBufferOwnership( cmd, lightBuf, VKTR_QUEUETYPE_COMPUTE, VKTR_QUEUETYPE_GRAPHICS );
		
		VKTR_CMD_StartRenderPass( cmd, mainRenderPass, mainFrameBuffer );
		
		VKTR_CMD_BindPipeline( cmd, mainPipeline );
		
		VKTR_CMD_BindVertexBuffer( cmd, vtxBuf, 0 );
		VKTR_CMD_BindVertexBuffer( cmd, nrmBuf, 1 );
		VKTR_CMD_BindVertexBuffer( cmd, uvBuf, 2 );
		
		VKTR_CMD_BindIndexBuffer( cmd, idxBuf, VKTR_INDEXTYPE_U32 );
		
		VKTR_CMD_BindDescriptorSet( cmd, mainDescriptorSet );
		
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_VERTEX, 0, 16 * 4, &vp );
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 64, 12, &camPos );
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 76, 4, &numLights );
		
		u32 lightBufStride = lightCullTilesX;
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 84, 4, &lightBufStride );
		
		//VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 88, 4, &shadowTextureIndex );
		
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 108, 4, &shadowDist1 );
		
		vec4 sunColour = (vec4){ 0.01, 0.009, 0.008 };
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 96, 12, &sunColour );
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 112, 12, &sunDir );
		
		// opaque pass
		for( int i = 0; i < numDrawBatches; i++ ) {
			u32 tIdx = drawBatches[i][2];
			//if( tIdx == 2 ) {
				//tIdx = 0;
			//}
			if( !transparentMaterials[tIdx] ) {
				VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 80, 4, &tIdx );
				VKTR_CMD_DrawIndexedOffset( cmd, drawBatches[i][0] * 3, drawBatches[i][1] * 3, 0 );
			}
		}
		
		// transparent pass
		for( int i = 0; i < numDrawBatches; i++ ) {
			u32 tIdx = drawBatches[i][2];
			//if( tIdx == 2 ) {
				//tIdx = 0;
			//}
			if( transparentMaterials[tIdx] ) {
				VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 80, 4, &tIdx );
				VKTR_CMD_DrawIndexedOffset( cmd, drawBatches[i][0] * 3, drawBatches[i][1] * 3, 0 );
			}
		}
		
		VKTR_CMD_EndRenderPass( cmd );
		
		VKTR_CMD_StartRenderPass( cmd, postRenderPass, postFrameBuffers[img] );
		VKTR_CMD_BindPipeline( cmd, postPipeline );
		
		VKTR_CMD_BindDescriptorSet( cmd, postDescriptorSet );
		VKTR_CMD_Draw( cmd, 3 );
		
		VKTR_CMD_EndRenderPass( cmd );
		VKTR_EndRecording( cmd );
		
		//printf( "MAINEND\n" );
		//fflush( stdout );
		
		//VKTR_Submit( queue, cmd, imageAvailable, renderFinished );
		VKTR_Semaphore renderSemaphores[2] = { lightCullSemaphore, shadowSemaphore };
		VKTR_Submit( queue, cmd, 2, renderSemaphores, 1, &renderFinished, frameCmdFences[frameCmdIndex] );
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameSubmit );
		
		//printf( "FRAMELIMIT\n" );
		//fflush( stdout );
		
		float frameTimeMS;
		do {
			s64 currentTime;
			QueryPerformanceCounter( (LARGE_INTEGER*)&currentTime );
			//s64 frameTime = currentTime - frameStart;
			s64 frameTime = currentTime - frameLimiterDelay;
			frameTimeMS = (float)frameTime / counterFreq * 1000.0f;
			
			//printf( "FRAMELIMIT ITER %f\n", frameTimeMS );
			//fflush( stdout );
			
			if( frameTimeMS < frameTimeLimitMS ) {
				YieldProcessor();
			}
		} while( frameTimeMS < frameTimeLimitMS );
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameLimiterDelay );
		
		//printf( "UPDATE\n" );
		//fflush( stdout );
		
		VKTR_Update( queue, window, renderFinished );
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameEnd );
		
		s64 frameTotal = frameEnd - frameStart;
		s64 preDelayTime = framePreDelay - frameStart;
		s64 acquireTime = frameAcquire - framePreDelay;
		s64 submitTime = frameSubmit - frameAcquire;
		s64 limiterTime = frameLimiterDelay - frameSubmit;
		s64 updateTime = frameEnd - frameLimiterDelay;
		
		float frameTotalMS = (float)frameTotal / counterFreq * 1000.0f;
		float preDelayTimeMS = (float)preDelayTime / counterFreq * 1000.0f;
		float acquireTimeMS = (float)acquireTime / counterFreq * 1000.0f;
		float submitTimeMS = (float)submitTime / counterFreq * 1000.0f;
		float limiterTimeMS = (float)limiterTime / counterFreq * 1000.0f;
		float updateTimeMS = (float)updateTime / counterFreq * 1000.0f;
		
		float delayMS = preDelayTimeMS + limiterTimeMS;
		
		float frameProcessTimeMS = frameTotalMS - delayMS;
		float currentFrameProcessTimeVariance = fabsf( frameProcessTimeMS - lastFrameProcessTimeMS );
		frameProcessTimeVarianceMS = fmaxf( frameProcessTimeVarianceMS * 0.95f, currentFrameProcessTimeVariance );
		
		frameTimeSlackMS = max( acquireTimeMS + delayMS - preDelayMarginMS - frameProcessTimeVarianceMS, 0 );
		
		lastFrameProcessTimeMS = frameProcessTimeMS;
		
		//printf( "\033[1H" );
		
		//printf( "\033[0;31mframe %d: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d, %.2f\n\033[0m", frame, frameTotalMS, preDelayTimeMS, acquireTimeMS, submitTimeMS, limiterTimeMS, updateTimeMS, frameTimeSlackMS, frameProcessTimeVarianceMS );
		printf( "\033[0;31m" );
		printf( "frame %d:\n", frame );
		printf( "  total: %.2f ms\n", frameTotalMS );
		printf( "  preDelay: %.2f ms\n", preDelayTimeMS );
		printf( "  acquire: %.2f ms\n", acquireTimeMS );
		printf( "  submit: %.2f ms\n", submitTimeMS );
		printf( "  limiter: %.2f ms\n", limiterTimeMS );
		printf( "  update: %.2f ms\n", updateTimeMS );
		printf( "  slack: %d ms\n", frameTimeSlackMS );
		printf( "  variance: %.2f ms\n", frameProcessTimeVarianceMS );
		printf( "\033[0m" );
		
		frame++;
	}
	
	if( config.display.timerPeriod > 0 ) {
		timeEndPeriod( config.display.timerPeriod );
	}
	
	if( config.display.useMMCSS ) {
		AvRevertMmThreadCharacteristics( MMCSS_TaskHandle );
	}
	
	return 0;
}
