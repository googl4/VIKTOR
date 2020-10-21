#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

#include "testShaders.h"
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
	VKTR_Queue uploadQueue = VKTR_GetQueue( VKTR_QUEUETYPE_TRANSFER );
	
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
	
	VKTR_CommandBuffer cmd = VKTR_CreateCommandBuffer( VKTR_QUEUETYPE_GRAPHICS );
	
	VKTR_Shader vertShader = VKTR_LoadShader( vertShaderData, vertShaderSize );
	VKTR_Shader fragShader = VKTR_LoadShader( fragShaderData, fragShaderSize );
	
	VKTR_Shader vertShader2 = VKTR_LoadShader( vertShaderData2, vertShaderSize2 );
	VKTR_Shader fragShader2 = VKTR_LoadShader( fragShaderData2, fragShaderSize2 );
	
	VKTR_Image depthBuf = VKTR_CreateImage( config.display.renderWidth, config.display.renderHeight, VKTR_FORMAT_D32F, VKTR_IMAGEFLAG_RENDERTARGET );
	VKTR_Image renderBuf = VKTR_CreateImage( config.display.renderWidth, config.display.renderHeight, VKTR_FORMAT_RGBA16F, VKTR_IMAGEFLAG_RENDERTARGET );
	
	//VKTR_Sampler sampler = VKTR_CreateSampler( VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_WRAP_CLAMP, 0 );
	VKTR_Sampler sampler = VKTR_CreateSampler( VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_FILTER_LINEAR, VKTR_WRAP_REPEAT, 0 );
	
	int numSwapchainImages = VKTR_GetSwapchainLength( window );
	
	VKTR_RenderPass mainRenderPass = VKTR_CreateRenderPass( 2, 1 );
	VKTR_SetRenderPassAttachment( mainRenderPass, 0, renderBuf, FALSE, TRUE, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_SHADER_READ );
	VKTR_SetRenderPassAttachment( mainRenderPass, 1, depthBuf, FALSE, TRUE, VKTR_IMAGELAYOUT_UNDEFINED, VKTR_IMAGELAYOUT_SHADER_READ );
	VKTR_SetSubPassOutput( mainRenderPass, 0, 0, 0 );
	VKTR_SetSubPassDepthAttachment( mainRenderPass, 0, 1 );
	VKTR_BuildRenderPass( mainRenderPass );
	
	VKTR_DescriptorPool descriptorPool = VKTR_CreateDescriptorPool( 64, 8, 0 );
	
	VKTR_Pipeline mainPipeline = VKTR_CreatePipeline( mainRenderPass, 0, 3, 1 );
	VKTR_PipelineSetShader( mainPipeline, VKTR_SHADERSTAGE_VERTEX, vertShader );
	VKTR_PipelineSetShader( mainPipeline, VKTR_SHADERSTAGE_FRAGMENT, fragShader );
	VKTR_PipelineSetDepthFunction( mainPipeline, VKTR_DEPTHFUNCTION_GT );
	VKTR_PipelineSetDepthEnable( mainPipeline, TRUE, TRUE );
	VKTR_PipelineSetBlend( mainPipeline, 0, VKTR_BLENDOP_ADD, VKTR_BLEND_SRC_ALPHA, VKTR_BLEND_SRC_ALPHA_INVERSE );
	VKTR_PipelineSetBlendEnable( mainPipeline, 0, TRUE );
	VKTR_PipelineSetVertexBinding( mainPipeline, 0, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( mainPipeline, 1, 0, 12, VKTR_FORMAT_RGB32F );
	VKTR_PipelineSetVertexBinding( mainPipeline, 2, 0, 8, VKTR_FORMAT_RG32F );
	VKTR_PipelineSetPushConstRange( mainPipeline, 0, 64, VKTR_SHADERSTAGE_VERTEX );
	VKTR_PipelineSetPushConstRange( mainPipeline, 64, 20, VKTR_SHADERSTAGE_FRAGMENT );
	VKTR_PipelineSetDescriptorBinding( mainPipeline, 0, 0, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 64 );
	VKTR_PipelineSetImmutableSamplerBinding( mainPipeline, 0, 1, VKTR_SHADERSTAGE_FRAGMENT, sampler );
	VKTR_BuildPipeline( mainPipeline );
	
	VKTR_DescriptorSet mainDescriptorSet = VKTR_CreateDescriptorSet( descriptorPool, mainPipeline, 0 );
	
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
		VKTR_PipelineSetShader( pipelines[i], VKTR_SHADERSTAGE_FRAGMENT, fragShader2 );
		VKTR_PipelineSetDepthEnable( pipelines[i], FALSE, FALSE );
		VKTR_PipelineSetBlendEnable( pipelines[i], 0, FALSE );
		VKTR_PipelineSetDescriptorBinding( pipelines[i], 0, 0, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 1 );
		VKTR_PipelineSetDescriptorBinding( pipelines[i], 0, 1, VKTR_SHADERSTAGE_FRAGMENT, VKTR_DESCRIPTOR_IMAGE, 1 );
		VKTR_PipelineSetImmutableSamplerBinding( pipelines[i], 0, 2, VKTR_SHADERSTAGE_FRAGMENT, sampler );
		VKTR_BuildPipeline( pipelines[i] );
		
		descriptorSets[i] = VKTR_CreateDescriptorSet( descriptorPool, pipelines[i], 0 );
		VKTR_SetTextureBinding( descriptorSets[i], 0, 0, renderBuf );
		VKTR_SetTextureBinding( descriptorSets[i], 1, 0, depthBuf );
	}
	
	VKTR_Semaphore imageAvailable = VKTR_CreateSemaphore();
	VKTR_Semaphore renderFinished = VKTR_CreateSemaphore();
	
	tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
	
	// https://github.com/zeux/meshoptimizer
	
	tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "assets/crytek-sponza/sponza-tri.obj", "assets/crytek-sponza/" );
	
	VKTR_Image textures[64];
	//VKTR_CommandBuffer uploadCmd = VKTR_CreateCommandBuffer( VKTR_QUEUETYPE_TRANSFER );
	VKTR_CommandBuffer uploadCmd = VKTR_CreateCommandBuffer( VKTR_QUEUETYPE_GRAPHICS );
	VKTR_StartRecording( uploadCmd );
	
	int transparentMaterials[64];
	
	for( int i = 0; i < materials.size(); i++ ) {
		if( materials[i].diffuse_texname.length() > 0 ) {
			std::string pathStr = "assets/crytek-sponza/" + materials[i].diffuse_texname;
			const char* texname = pathStr.c_str();
			printf( "mat %d: %s ", i, texname );
			int w, h, c;
			u8* imgData = stbi_load( texname, &w, &h, &c, 4 );
			printf( "%dx%d\n", w, h );
			
			textures[i] = VKTR_CreateImage( w, h, VKTR_FORMAT_RGBA8_SRGB, (VKTR_ImageFlags)0 );
			transparentMaterials[i] = ( c == 4 );
			
			VKTR_SetTextureBinding( mainDescriptorSet, 0, i, textures[i] );
			
			VKTR_Buffer uploadBuffer = VKTR_CreateBuffer( w * h * 4, VKTR_BUFFERTYPE_STAGING );
			void* uploadPtr = VKTR_MapBuffer( uploadBuffer );
			memcpy( uploadPtr, imgData, w * h * 4 );
			
			VKTR_CMD_CopyToImage( uploadCmd, uploadBuffer, textures[i] );
		}
	}
	
	VKTR_EndRecording( uploadCmd );
	VKTR_Submit( queue, uploadCmd, VKTR_INVALID_HANDLE, VKTR_INVALID_HANDLE );
	
	int numVerts = shapes[0].mesh.indices.size();
	
	VKTR_Buffer vtxBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer nrmBuf = VKTR_CreateBuffer( numVerts * 12, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer uvBuf = VKTR_CreateBuffer( numVerts * 8, VKTR_BUFFERTYPE_DEVICE_VERTEX );
	VKTR_Buffer idxBuf = VKTR_CreateBuffer( numVerts * 4, VKTR_BUFFERTYPE_DEVICE_INDEX );
	
	vec3* vtxPtr = (vec3*)VKTR_MapBuffer( vtxBuf );
	vec3* nrmPtr = (vec3*)VKTR_MapBuffer( nrmBuf );
	vec2* uvPtr = (vec2*)VKTR_MapBuffer( uvBuf );
	u32* idxPtr = (u32*)VKTR_MapBuffer( idxBuf );
	
	u32 drawBatches[256][3] = {};
	int numDrawBatches = 0;
	int currentMaterial = shapes[0].mesh.material_ids[0];
	
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
		
		int material = shapes[0].mesh.material_ids[i/3];
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
	
	mat4 projection = mat4_perspectiveReverseZ( 35.0f / 57.2958f, (float)config.display.displayWidth / config.display.displayHeight, 0.1f );
	projection = mat4_scale_aniso( projection, (vec4){ 1, -1, 1, 1 } );
	
	int frame = 0;
	
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
	
	while( !VKTR_WindowClosed( window ) ) {
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameStart );
		
		if( frameTimeSlackMS > 0 && config.display.enablePreDelay ) {
			Sleep( frameTimeSlackMS );
		}
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&framePreDelay );
		
		u32 img = VKTR_GetNextImage( window, imageAvailable );
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameAcquire );
		
		mat4 view = mat4_lookat( (vec4){ 0.0f, 100.0f, 0.0f }, (vec4){ 1, 100, 0 }, (vec4){ 0, 1, 0 } );
		mat4 vp = mat4_mul( projection, view );
		
		vec4 lightDir = vec4_norm( (vec4){ 0.1, -1, 0.1 } );
		
		VKTR_StartRecording( cmd );
		VKTR_CMD_StartRenderPass( cmd, mainRenderPass );
		VKTR_CMD_BindPipeline( cmd, mainPipeline );
		
		VKTR_CMD_BindVertexBuffer( cmd, vtxBuf, 0 );
		VKTR_CMD_BindVertexBuffer( cmd, nrmBuf, 1 );
		VKTR_CMD_BindVertexBuffer( cmd, uvBuf, 2 );
		
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_VERTEX, 0, 16 * 4, &vp );
		VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 64, 16, &lightDir );
		
		VKTR_CMD_BindDescriptorSet( cmd, mainDescriptorSet );
		
		VKTR_CMD_BindIndexBuffer( cmd, idxBuf, VKTR_INDEXTYPE_U32 );
		
		// opaque pass
		for( int i = 0; i < numDrawBatches; i++ ) {
			u32 tIdx = drawBatches[i][2];
			if( tIdx == 2 ) {
				tIdx = 0;
			}
			if( !transparentMaterials[tIdx] ) {
				VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 80, 4, &tIdx );
				VKTR_CMD_DrawIndexedOffset( cmd, drawBatches[i][0] * 3, drawBatches[i][1] * 3, 0 );
			}
		}
		
		// transparent pass
		for( int i = 0; i < numDrawBatches; i++ ) {
			u32 tIdx = drawBatches[i][2];
			if( tIdx == 2 ) {
				tIdx = 0;
			}
			if( transparentMaterials[tIdx] ) {
				VKTR_CMD_PushConst( cmd, VKTR_SHADERSTAGE_FRAGMENT, 80, 4, &tIdx );
				VKTR_CMD_DrawIndexedOffset( cmd, drawBatches[i][0] * 3, drawBatches[i][1] * 3, 0 );
			}
		}
		
		VKTR_CMD_EndRenderPass( cmd );
		VKTR_CMD_StartRenderPass( cmd, renderPasses[img] );
		VKTR_CMD_BindPipeline( cmd, pipelines[img] );
		
		VKTR_CMD_BindDescriptorSet( cmd, descriptorSets[img] );
		VKTR_CMD_Draw( cmd, 3 );
		
		VKTR_CMD_EndRenderPass( cmd );
		VKTR_EndRecording( cmd );
		VKTR_Submit( queue, cmd, imageAvailable, renderFinished );
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameSubmit );
		
		float frameTimeMS;
		do {
			s64 currentTime;
			QueryPerformanceCounter( (LARGE_INTEGER*)&currentTime );
			//s64 frameTime = currentTime - frameStart;
			s64 frameTime = currentTime - frameLimiterDelay;
			frameTimeMS = (float)frameTime / counterFreq * 1000.0f;
			YieldProcessor();
		} while( frameTimeMS < frameTimeLimitMS );
		
		QueryPerformanceCounter( (LARGE_INTEGER*)&frameLimiterDelay );
		
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
		
		printf( "\033[0;31mframe %d: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d, %.2f\n\033[0m", frame, frameTotalMS, preDelayTimeMS, acquireTimeMS, submitTimeMS, limiterTimeMS, updateTimeMS, frameTimeSlackMS, frameProcessTimeVarianceMS );
		
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
