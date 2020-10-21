#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "windows.h"

//#define VK_USE_PLATFORM_WIN32_KHR
#include "volk.h"

#include "types.h"
#include "viktor.h"

#define VK_COLOR_COMPONENT_RG_BITS VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
#define VK_COLOR_COMPONENT_RGB_BITS VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
#define VK_COLOR_COMPONENT_RGBA_BITS VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT

const VkFormat formatMap[] = {
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_D32_SFLOAT
};

VkShaderStageFlagBits stageMap[] = {
	VK_SHADER_STAGE_VERTEX_BIT,
	VK_SHADER_STAGE_FRAGMENT_BIT,
	VK_SHADER_STAGE_COMPUTE_BIT,
	VK_SHADER_STAGE_GEOMETRY_BIT
};

VkDescriptorType descriptorMap[] = {
	VK_DESCRIPTOR_TYPE_SAMPLER,
	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
};

VkIndexType indexMap[] = {
	VK_INDEX_TYPE_UINT16,
	VK_INDEX_TYPE_UINT32
};

// TODO replace malloc

typedef struct {
	HWND osHandle;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	u32 numSwapchainImages;
	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	u32 imageIndex;
	int w, h;
} VKTR_Internal_Window;

typedef struct {
	u32 numAttachments;
	u32 numSubPasses;
	VkAttachmentDescription attachments[8];
	u32 subPassInputs[8][8];
	u32 subPassOutputs[8][8];
	u32 subPassDepth[8];
	int subPassAttachmentCounts[8];
	VkRenderPass vkRenderPass;
	VkImageView vkFBImageViews[8];
	VkFramebuffer vkFramebuffer;
	int FBWidth;
	int FBHeight;
} VKTR_Internal_RenderPass;

typedef struct {
	VKTR_Internal_RenderPass* renderPass;
	u32 subPass;
	VkPipelineShaderStageCreateInfo stageCreateInfo[4];
	int numStages;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkVertexInputBindingDescription vertexBindings[8];
	VkVertexInputAttributeDescription vertexAttributes[8];
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineColorBlendAttachmentState colourBlendAttachments[8];
	VkPipelineColorBlendStateCreateInfo colourBlending;
	VkPipelineDepthStencilStateCreateInfo depthState;
	VkPushConstantRange pushConsts[4];
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayoutBinding descriptorLayoutBindings[8][8];
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo[8];
    VkDescriptorSetLayout descriptorLayouts[8];
    int numDescriptorLayouts;
	VkPipeline pipeline;
} VKTR_Internal_Pipeline;

typedef struct {
	VkBuffer buffer;
	VkDeviceMemory memory;
	size_t size;
	void* mapPtr;
} VKTR_Internal_Buffer;

typedef struct {
	VkImage tex;
	VkDeviceMemory texMemory;
	size_t memorySize;
	VkImageView defaultView;
	VKTR_Format format;
	int w, h;
} VKTR_Internal_Image;


VkInstance instance;
VkDevice device;
VkPhysicalDevice physicalDevice;

int graphicsQueueFamily = -1;
int computeQueueFamily = -1;
int transferQueueFamily = -1;

//VkCommandPool commandPools[3];


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData  ) {
	printf( "Layer: %s\n", pCallbackData->pMessage );
	fflush( stdout );
	
	//exit( 0 );
	
	return VK_FALSE;
}

void registerDebugCallback( VkInstance instance ) {
	VkDebugUtilsMessengerEXT callback;
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//debugCreateInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	
	VkDebugUtilsMessengerEXT callbackMessenger;
	vkCreateDebugUtilsMessengerEXT( instance, &debugCreateInfo, NULL, &callbackMessenger );
}

void createDevice( VkInstance instance, VkPhysicalDevice* physicalDevice, VkDevice* device ) {
	const char* deviceExtensions[2] = {
		"VK_KHR_swapchain",
		"VK_EXT_inline_uniform_block",
		//"VK_KHR_multiview",
		//"VK_NV_viewport_array2",
		//"VK_NVX_multiview_per_view_attributes",
		//"VK_EXT_sampler_filter_minmax",
		//"VK_NV_dedicated_allocation"
	};

	const char* deviceLayers[1] = {
		"VK_LAYER_LUNARG_validation"
	};
	
	u32 devices = 1;
	vkEnumeratePhysicalDevices( instance, &devices, physicalDevice );
	
	u32 numQueueFamilies = 8;
	VkQueueFamilyProperties queueFamilies[8];
	vkGetPhysicalDeviceQueueFamilyProperties( *physicalDevice, &numQueueFamilies, queueFamilies );
	
	printf( "Queue Families: %d\n", numQueueFamilies );
	for( int i = 0; i < numQueueFamilies; i++ ) {
		printf( "  %d: %d ", i, queueFamilies[i].queueCount );
		if( queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
			putchar( 'G' );
			if( graphicsQueueFamily < 0 ) {
				graphicsQueueFamily = i;
			}
		}
		if( queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT ) {
			putchar( 'C' );
			if( computeQueueFamily < 0 && i != graphicsQueueFamily ) {
				computeQueueFamily = i;
			}
		}
		if( queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT ) {
			putchar( 'T' );
			if( transferQueueFamily < 0 && i != graphicsQueueFamily && i != computeQueueFamily ) {
				transferQueueFamily = i;
			}
		}
		putchar( '\n' );
	}
	
	printf( "has async compute? %d\n", computeQueueFamily >= 0 );
	printf( "has async transfer? %d\n", transferQueueFamily >= 0 );
	
	float queuePriority = 1.0f;
	
	VkDeviceQueueCreateInfo queueCreateInfo[3];
	memset( queueCreateInfo, 0, 3 * sizeof( VkDeviceQueueCreateInfo ) );
	
	queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo[0].queueFamilyIndex = graphicsQueueFamily;
	queueCreateInfo[0].queueCount = 1;
	queueCreateInfo[0].pQueuePriorities = &queuePriority;
	
	int nQueues = 1;
	
	if( computeQueueFamily >= 0 ) {
		queueCreateInfo[nQueues].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo[nQueues].queueFamilyIndex = computeQueueFamily;
		queueCreateInfo[nQueues].queueCount = 1;
		queueCreateInfo[nQueues].pQueuePriorities = &queuePriority;
		nQueues++;
	}
	
	if( transferQueueFamily >= 0 ) {
		queueCreateInfo[nQueues].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo[nQueues].queueFamilyIndex = transferQueueFamily;
		queueCreateInfo[nQueues].queueCount = 1;
		queueCreateInfo[nQueues].pQueuePriorities = &queuePriority;
		nQueues++;
	}
	
	VkPhysicalDeviceFeatures deviceFeatures = {};
	
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = nQueues;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = 2;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceCreateInfo.enabledLayerCount = 1;
	deviceCreateInfo.ppEnabledLayerNames = deviceLayers;
	
	vkCreateDevice( *physicalDevice, &deviceCreateInfo, NULL, device );
	
	volkLoadDevice( *device );
}

void VKTR_Init( const char* appName ) {
	printf( "VIKTOR Initialisation start\n" );
	fflush( stdout );
	
	volkInitialize();
	
	u32 ver = volkGetInstanceVersion();
	printf( "Vulkan %d.%d.%d\n", VK_VERSION_MAJOR( ver ), VK_VERSION_MINOR( ver ), VK_VERSION_PATCH( ver ) );
	fflush( stdout );
	
	const char* instanceExtensions[4] = {
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
		//"VK_EXT_debug_report",
		"VK_EXT_debug_utils"
	};

	const char* instanceLayers[1] = {
		//"VK_LAYER_RENDERDOC_Capture",
		//"VK_LAYER_VALVE_steam_overlay",
		"VK_LAYER_KHRONOS_validation"
	};
	
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "VIKTOR";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_2;
	
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = 3;//4;
	createInfo.ppEnabledExtensionNames = instanceExtensions;
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = instanceLayers;
	
	VkResult ciRes = vkCreateInstance( &createInfo, NULL, &instance );
	printf( "vkCreateInstance returned %d\n", ciRes );
	fflush( stdout );
	
	volkLoadInstanceOnly( instance );
	
	registerDebugCallback( instance );
	
	createDevice( instance, &physicalDevice, &device );
	
	/*
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = graphicsQueueFamily;
	poolInfo.flags = 0;//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	
	vkCreateCommandPool( device, &poolInfo, NULL, &commandPools[VKTR_QUEUETYPE_GRAPHICS] );
	
	poolInfo.queueFamilyIndex = computeQueueFamily >= 0 ? computeQueueFamily : graphicsQueueFamily;
	vkCreateCommandPool( device, &poolInfo, NULL, &commandPools[VKTR_QUEUETYPE_COMPUTE] );
	
	poolInfo.queueFamilyIndex = transferQueueFamily >= 0 ? transferQueueFamily : graphicsQueueFamily;
	vkCreateCommandPool( device, &poolInfo, NULL, &commandPools[VKTR_QUEUETYPE_TRANSFER] );
	*/
}

VKTR_Queue VKTR_GetQueue( VKTR_QueueType type ) {
	int queues[3] = {
		graphicsQueueFamily,
		computeQueueFamily,
		transferQueueFamily
	};
	
	if( queues[type] < 0 ) {
		type = VKTR_QUEUETYPE_GRAPHICS;
	}
	
	VkQueue queue;
	vkGetDeviceQueue( device, queues[type], 0, &queue );
	return queue;
}

LRESULT CALLBACK WndProc( HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	switch( uMsg ) {
		case WM_CLOSE:
			//printf( "WM_CLOSE\n" );
			break;
			
		case WM_DESTROY:
			//printf( "WM_DESTROY\n" );
			PostQuitMessage(0); 
			break;
	}
	
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

void createImageView( VkDevice device, VkImageView* view, VkImage image, VkFormat format, VkImageAspectFlagBits aspectBits ) {
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask = aspectBits;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	vkCreateImageView( device, &viewCreateInfo, NULL, view );
}

VKTR_Window VKTR_CreateWindow( u32 width, u32 height, const char* title, VKTR_WindowFlags flags ) {
	HINSTANCE hInst = GetModuleHandle( NULL );
	WNDCLASS wndClass = {};
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInst;
	wndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	wndClass.hbrBackground = GetStockObject( BLACK_BRUSH );
	wndClass.lpszClassName = title;
	RegisterClass( &wndClass );
	
	DWORD styleFlags = WS_VISIBLE;
	int windowPosX = 0;
	int windowPosY = 0;
	if( ~flags & VKTR_WINDOWFLAG_BORDERLESS ) {
		styleFlags |= WS_SYSMENU | WS_OVERLAPPEDWINDOW;
		windowPosX = CW_USEDEFAULT;
		windowPosY = CW_USEDEFAULT;
	} else {
		styleFlags |= WS_POPUP;
	}
	
	RECT bounds = { 0, 0, width, height };
	AdjustWindowRect( &bounds, styleFlags, FALSE );
	printf( "adjusted window bounds: %d, %d, %d, %d\n", bounds.left, bounds.top, bounds.right, bounds.bottom );
	
	HWND hWnd = CreateWindow( title, title, styleFlags, windowPosX, windowPosY, bounds.right - bounds.left, bounds.bottom - bounds.top, NULL, NULL, hInst, NULL );
	
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hwnd = hWnd;
	surfaceCreateInfo.hinstance = hInst;
	
	VkSurfaceKHR surface;
	vkCreateWin32SurfaceKHR( instance, &surfaceCreateInfo, NULL, &surface );
	
	
	VkBool32 presentSupport;
	vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, 0, surface, &presentSupport );
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &surfaceCaps );
	
	
	int minImages = flags & VKTR_WINDOWFLAG_TRIPLEBUFFER ? 3 : 2;
	//VkPresentModeKHR presentMode = flags & VKTR_WINDOWFLAG_VSYNC ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
	VkPresentModeKHR presentMode = flags & VKTR_WINDOWFLAG_VSYNC ? VK_PRESENT_MODE_FIFO_RELAXED_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
	
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = minImages;
	swapchainCreateInfo.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent.width = width;
	swapchainCreateInfo.imageExtent.height = height;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	//swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	//swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR;
	swapchainCreateInfo.preTransform = surfaceCaps.currentTransform;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	
	VkSwapchainKHR swapchain;
	vkCreateSwapchainKHR( device, &swapchainCreateInfo, NULL, &swapchain );
	
	
	u32 numSwapchainImages;
	vkGetSwapchainImagesKHR( device, swapchain, &numSwapchainImages, NULL );
	VkImage* swapchainImages = malloc( numSwapchainImages * sizeof( VkImage ) );
	vkGetSwapchainImagesKHR( device, swapchain, &numSwapchainImages, swapchainImages );
	printf( "swapchain size: %d\n", numSwapchainImages );
	
	VkImageView* swapchainImageViews = malloc( numSwapchainImages * sizeof( VkImageView ) );
	for( int i = 0; i < numSwapchainImages; i++ ) {
		createImageView( device, &swapchainImageViews[i], swapchainImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT );
	}
	
	VKTR_Internal_Window* window = malloc( sizeof( VKTR_Internal_Window ) );
	window->osHandle = hWnd;
	window->surface = surface;
	window->swapchain = swapchain;
	window->numSwapchainImages = numSwapchainImages;
	window->swapchainImages = swapchainImages;
	window->swapchainImageViews = swapchainImageViews;
	window->w = width;
	window->h = height;
	
	return window;
}

int VKTR_WindowClosed( VKTR_Window window ) {
	MSG winMsg;
	PeekMessage( &winMsg, NULL, 0, 0, PM_REMOVE );
	DispatchMessage( &winMsg );
	
	if( winMsg.message == WM_QUIT ) {
		return TRUE;
	}
	
	return FALSE;
}

void VKTR_Update( VKTR_Queue queue, VKTR_Window window, VKTR_Semaphore waitSemaphore ) {
	VKTR_Internal_Window* win = window;
	
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = (VkSemaphore*)&waitSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &win->swapchain;
	presentInfo.pImageIndices = &win->imageIndex;
	
	//vkQueueWaitIdle( queue );
	
	vkQueuePresentKHR( queue, &presentInfo );
	
	//vkQueueWaitIdle( queue );
}

int VKTR_GetSwapchainLength( VKTR_Window window ) {
	VKTR_Internal_Window* win = window;
	
	return win->numSwapchainImages;
}

VKTR_Image VKTR_GetSwapchainImage( VKTR_Window window, u32 n ) {
	VKTR_Internal_Window* win = window;
	
	VKTR_Internal_Image* img = malloc( sizeof( VKTR_Internal_Image ) );
	img->tex = win->swapchainImages[n];
	//VkDeviceMemory texMemory;
	//size_t memorySize;
	img->defaultView = win->swapchainImageViews[n];
	img->format = VKTR_FORMAT_RGBA8_SRGB;
	img->w = win->w;
	img->h = win->h;
	
	return img;
}

u32 VKTR_GetNextImage( VKTR_Window window, VKTR_Semaphore availableSemaphore ) {
	VKTR_Internal_Window* win = window;
	
	vkAcquireNextImageKHR( device, win->swapchain, UINT64_MAX, availableSemaphore, VK_NULL_HANDLE, &win->imageIndex );
	
	//vkDeviceWaitIdle( device );
	
	return win->imageIndex;
}

VKTR_Semaphore VKTR_CreateSemaphore( void ) {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkSemaphore semaphore;
	vkCreateSemaphore( device, &semaphoreInfo, NULL, &semaphore );
	
	return semaphore;
}

//void VKTR_DestroySemaphore( VKTR_Semaphore semaphore );

VKTR_Buffer VKTR_CreateBuffer( u64 size, VKTR_BufferType type ) {
	VKTR_Internal_Buffer* buf = malloc( sizeof( VKTR_Internal_Buffer ) );
	
	VkBufferUsageFlagBits usageFlags[] = {
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 	//VKTR_BUFFERTYPE_DEVICE_GENERIC,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 	//VKTR_BUFFERTYPE_DEVICE_VERTEX,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 	//VKTR_BUFFERTYPE_DEVICE_INDEX,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 	//VKTR_BUFFERTYPE_DEVICE_UNIFORM,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 	//VKTR_BUFFERTYPE_HOST,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 										//VKTR_BUFFERTYPE_STAGING
	};
	
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags[type];
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	//VkBuffer vertexBuffer;
	vkCreateBuffer( device, &bufferInfo, NULL, &buf->buffer );
	
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements( device, buf->buffer, &memRequirements );
	
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );
	
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = 2;//findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	//VkDeviceMemory vertexBufferMemory;
	vkAllocateMemory( device, &allocInfo, NULL, &buf->memory );
	
	vkBindBufferMemory( device, buf->buffer, buf->memory, 0 );
	
	buf->size = size;
	
	return buf;
}

void* VKTR_MapBuffer( VKTR_Buffer buffer ) {
	VKTR_Internal_Buffer* buf = buffer;
	
	void* data;
	vkMapMemory( device, buf->memory, 0, buf->size, 0, &data );
	
	buf->mapPtr = data;
	
	return data;
}

VKTR_Image VKTR_CreateImage( u32 width, u32 height, VKTR_Format type, VKTR_ImageFlags flags ) {
	VKTR_Internal_Image* img = malloc( sizeof( VKTR_Internal_Image ) );
	
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = formatMap[type];
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;
	
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	
	if( flags & VKTR_IMAGEFLAG_RENDERTARGET ) {
		if( type == VKTR_FORMAT_D32F ) {
			imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		} else {
			imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
	}
	
	if( flags & VKTR_IMAGEFLAG_SUBPASSINPUT ) {
		imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
	
	vkCreateImage( device, &imageInfo, NULL, &img->tex );
	
	VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements( device, img->tex, &memRequirements );

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = 0;

	vkAllocateMemory( device, &allocInfo, NULL, &img->texMemory );

    vkBindImageMemory( device, img->tex, img->texMemory, 0 );
    
    img->memorySize = allocInfo.allocationSize;
    
    createImageView( device, &img->defaultView, img->tex, formatMap[type], type == VKTR_FORMAT_D32F ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT );
    
    img->format = type;
    img->w = width;
    img->h = height;
    
    return img;
}

//VKTR_Buffer VKTR_GetImageBuffer( VKTR_Image image )
//void VKTR_FreeImage( VKTR_Image image )

VKTR_Shader VKTR_LoadShader( const void* data, u64 dataSize ) {
	VkShaderModuleCreateInfo shaderCreateInfo = {};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = dataSize;
	shaderCreateInfo.pCode = data;
	
	VkShaderModule shaderModule;
	vkCreateShaderModule( device, &shaderCreateInfo, NULL, &shaderModule );
	
	return shaderModule;
}

VKTR_CommandPool VKTR_CreateCommandPool( VKTR_QueueType queueType ) {
	int queues[3] = {
		graphicsQueueFamily,
		computeQueueFamily,
		transferQueueFamily
	};
	
	if( queues[type] < 0 ) {
		type = VKTR_QUEUETYPE_GRAPHICS;
	}
	
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = type;
	poolInfo.flags = 0;//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	
	vkCreateCommandPool( device, &poolInfo, NULL, &commandPools[queueType] );
}

VKTR_CommandBuffer VKTR_CreateCommandBuffer( VKTR_CommandPool pool ) {
	VkCommandBuffer cmdBuf;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers( device, &allocInfo, &cmdBuf );
	
	return cmdBuf;
}

void VKTR_StartRecording( VKTR_CommandBuffer buffer ) {
	//vkResetCommandBuffer( buffer, 0 );
	
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
	vkBeginCommandBuffer( buffer, &beginInfo );
}

void VKTR_EndRecording( VKTR_CommandBuffer buffer ) {
	vkEndCommandBuffer( buffer );
}

void VKTR_Submit( VKTR_Queue queue, VKTR_CommandBuffer buffer, VKTR_Semaphore waitSemaphore, VKTR_Semaphore signalSemaphore ) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = waitSemaphore != VKTR_INVALID_HANDLE;
	submitInfo.pWaitSemaphores = (VkSemaphore*)&waitSemaphore;
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = (VkCommandBuffer*)&buffer;
	submitInfo.signalSemaphoreCount = signalSemaphore != VKTR_INVALID_HANDLE;
	submitInfo.pSignalSemaphores = (VkSemaphore*)&signalSemaphore;
	
	vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
}

VKTR_RenderPass VKTR_CreateRenderPass( u32 numAttachments, u32 numSubPasses ) {
	VKTR_Internal_RenderPass* renderPass = malloc( sizeof( VKTR_Internal_RenderPass ) );
	memset( renderPass, 0, sizeof( VKTR_Internal_RenderPass ) );
	
	renderPass->numAttachments = numAttachments;
	renderPass->numSubPasses = numSubPasses;
	
	for( int i = 0; i < 8; i++ ) {
		for( int j = 0; j < 8; j++ ) {
			renderPass->subPassInputs[i][j] = VK_ATTACHMENT_UNUSED;
			renderPass->subPassOutputs[i][j] = VK_ATTACHMENT_UNUSED;
		}
		
		renderPass->subPassDepth[i] = VK_ATTACHMENT_UNUSED;
	}
	
	renderPass->FBWidth = INT_MAX;
	renderPass->FBHeight = INT_MAX;
	
	return renderPass;
}

VkImageLayout imageLayoutMap[] = {
	VK_IMAGE_LAYOUT_UNDEFINED,
	VK_IMAGE_LAYOUT_GENERAL,
	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
};

void VKTR_SetRenderPassAttachment( VKTR_RenderPass renderPass, u32 attachment, VKTR_Image image, int loadInput, int storeOutput, VKTR_ImageLayout srcLayout, VKTR_ImageLayout dstLayout ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	VKTR_Internal_Image* img = image;
	
	memset( &rp->attachments[attachment], 0, sizeof( VkAttachmentDescription ) );
	
	VkAttachmentDescription* at = &rp->attachments[attachment];
	at->format = formatMap[img->format];
	at->samples = VK_SAMPLE_COUNT_1_BIT; // FIX
	at->loadOp = loadInput ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR; // FIX
	at->storeOp = storeOutput ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	at->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at->initialLayout = imageLayoutMap[srcLayout];
	at->finalLayout = imageLayoutMap[dstLayout];
	
	rp->vkFBImageViews[attachment] = img->defaultView;
	
	rp->FBWidth = min( rp->FBWidth, img->w );
	rp->FBHeight = min( rp->FBHeight, img->h );
}

void VKTR_SetSubPassInput( VKTR_RenderPass renderPass, u32 subPass, u32 attachment, u32 binding ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	rp->subPassInputs[subPass][binding] = attachment;
}

void VKTR_SetSubPassOutput( VKTR_RenderPass renderPass, u32 subPass, u32 attachment, u32 binding ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	rp->subPassOutputs[subPass][binding] = attachment;
}

void VKTR_SetSubPassDepthAttachment( VKTR_RenderPass renderPass, u32 subPass, u32 attachment ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	rp->subPassDepth[subPass] = attachment;
}

void VKTR_BuildRenderPass( VKTR_RenderPass renderPass ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	VkSubpassDescription subPasses[8];
	VkAttachmentReference attachmentRefs[8][8];
	VkAttachmentReference depthAttachmentRefs[8];
	VkAttachmentReference inputAttachmentRefs[8][8];
	
	memset( subPasses, 0, sizeof( subPasses ) );
	memset( attachmentRefs, 0, sizeof( attachmentRefs ) );
	memset( depthAttachmentRefs, 0, sizeof( depthAttachmentRefs ) );
	memset( inputAttachmentRefs, 0, sizeof( inputAttachmentRefs ) );
	
	VkSubpassDependency subPassDependencies[64];
	int numDependencies = 0;
	
	for( int i = 0; i < 8; i++ ) {
		subPasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPasses[i].pColorAttachments = attachmentRefs[i];
		subPasses[i].pInputAttachments = inputAttachmentRefs[i];
		
		subPasses[i].pDepthStencilAttachment = &depthAttachmentRefs[i];
		depthAttachmentRefs[i].attachment = rp->subPassDepth[i];
		depthAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		//depthAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_GENERAL;
		
		int numAttachments = 0;
		int numInputAttachments = 0;
		
		for( int j = 0; j < 8; j++ ) {
			attachmentRefs[i][j].attachment = VK_ATTACHMENT_UNUSED;
			attachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_UNDEFINED;
			
			if( rp->subPassInputs[i][j] != VK_ATTACHMENT_UNUSED ) {
				//inputAttachmentRefs[i][j].attachment = rp->subPassInputs[i][j];
				//inputAttachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				//inputAttachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_GENERAL;
				
				int hasDependency = FALSE;
				
				if( i > 0 ) {
					for( int k = i - 1; k >= 0; k++ ) {
						for( int l = 0; l < subPasses[k].colorAttachmentCount; l++ ) {
							if( rp->subPassInputs[i][j] == rp->subPassOutputs[k][l] ) {
								subPassDependencies[numDependencies].srcSubpass = k;
								subPassDependencies[numDependencies].dstSubpass = i;
								subPassDependencies[numDependencies].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
								subPassDependencies[numDependencies].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
								subPassDependencies[numDependencies].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
								subPassDependencies[numDependencies].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								subPassDependencies[numDependencies].dependencyFlags = 0; // VK_DEPENDENCY_BY_REGION_BIT;
								numDependencies++;
								
								hasDependency = TRUE;
								break;
							}
						}
						
						if( rp->subPassInputs[i][j] == rp->subPassDepth[k] ) {
							subPassDependencies[numDependencies].srcSubpass = k;
							subPassDependencies[numDependencies].dstSubpass = i;
							subPassDependencies[numDependencies].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							subPassDependencies[numDependencies].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
							subPassDependencies[numDependencies].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							subPassDependencies[numDependencies].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							subPassDependencies[numDependencies].dependencyFlags = 0; // VK_DEPENDENCY_BY_REGION_BIT;
							numDependencies++;
							
							hasDependency = TRUE;
						}
						
						if( hasDependency ) {
							break;
						}
					}
				}
				
				if( rp->attachments[rp->subPassInputs[i][j]].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ) {
					int firstLoad = TRUE;
					for( int k = 0; k < i && firstLoad; k++ ) {
						for( int l = 0; l < 8; l++ ) {
							if( rp->subPassInputs[k][l] != VK_ATTACHMENT_UNUSED && rp->subPassInputs[k][l] == rp->subPassInputs[i][j] ) {
								firstLoad = FALSE;
								break;
							}
							if( rp->subPassOutputs[k][l] != VK_ATTACHMENT_UNUSED && rp->subPassOutputs[k][l] == rp->subPassInputs[i][j] ) {
								firstLoad = FALSE;
								break;
							}
						}
					}
					
					if( firstLoad ) {
						subPassDependencies[numDependencies].srcSubpass = VK_SUBPASS_EXTERNAL;
						subPassDependencies[numDependencies].dstSubpass = i;
						subPassDependencies[numDependencies].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						subPassDependencies[numDependencies].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						subPassDependencies[numDependencies].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						subPassDependencies[numDependencies].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						subPassDependencies[numDependencies].dependencyFlags = 0; // VK_DEPENDENCY_BY_REGION_BIT;
						numDependencies++;
					}
				}
				
				if( j >= numInputAttachments ) {
					numInputAttachments = j + 1;
				}
			}
			
			if( rp->subPassOutputs[i][j] != VK_ATTACHMENT_UNUSED ) {
				attachmentRefs[i][j].attachment = rp->subPassOutputs[i][j];
				attachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				//attachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_GENERAL;
				
				if( j >= numAttachments ) {
					numAttachments = j + 1;
				}
				
				if( rp->attachments[rp->subPassOutputs[i][j]].storeOp == VK_ATTACHMENT_STORE_OP_STORE ) {
					int lastStore = TRUE;
					for( int k = i + 1; k < 8 && lastStore; k++ ) {
						for( int l = 0; l < 8; l++ ) {
							if( rp->subPassOutputs[k][l] != VK_ATTACHMENT_UNUSED && rp->subPassOutputs[k][l] == rp->subPassOutputs[i][j] ) {
								lastStore = FALSE;
								break;
							}
						}
					}
					
					if( lastStore ) {
						subPassDependencies[numDependencies].srcSubpass = i;
						subPassDependencies[numDependencies].dstSubpass = VK_SUBPASS_EXTERNAL;
						subPassDependencies[numDependencies].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						subPassDependencies[numDependencies].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						subPassDependencies[numDependencies].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						subPassDependencies[numDependencies].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						subPassDependencies[numDependencies].dependencyFlags = 0; // VK_DEPENDENCY_BY_REGION_BIT;
						numDependencies++;
					}
				}
			}
		}
		
		subPasses[i].colorAttachmentCount = numAttachments;
		rp->subPassAttachmentCounts[i] = numAttachments;
		
		//subPasses[i].inputAttachmentCount = numInputAttachments;
		subPasses[i].inputAttachmentCount = 0;
	}
	
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = rp->numAttachments;
	renderPassCreateInfo.pAttachments = rp->attachments;
	renderPassCreateInfo.subpassCount = rp->numSubPasses;
	renderPassCreateInfo.pSubpasses = subPasses;
	renderPassCreateInfo.dependencyCount = numDependencies;
	renderPassCreateInfo.pDependencies = subPassDependencies;
	
	vkCreateRenderPass( device, &renderPassCreateInfo, NULL, &rp->vkRenderPass );
	
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = rp->vkRenderPass;
	framebufferInfo.attachmentCount = rp->numAttachments;
	framebufferInfo.pAttachments = rp->vkFBImageViews;
	framebufferInfo.width = rp->FBWidth;
	framebufferInfo.height = rp->FBHeight;
	framebufferInfo.layers = 1;
	
	vkCreateFramebuffer( device, &framebufferInfo, NULL, &rp->vkFramebuffer );
}
/*
void VKTR_FreeRenderPass( VKTR_RenderPass renderPass ) {
	
}
*/

VKTR_Pipeline VKTR_CreatePipeline( VKTR_RenderPass renderPass, u32 subPass, int numVertexBindings, int numDescriptorLayouts ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	VKTR_Internal_Pipeline* pi = malloc( sizeof( VKTR_Internal_Pipeline ) );
	memset( pi, 0, sizeof( VKTR_Internal_Pipeline ) );
	
	pi->renderPass = rp;
	pi->subPass = subPass;
	
	pi->viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pi->viewportState.viewportCount = 1;
	pi->viewportState.pViewports = &pi->viewport;
	pi->viewportState.scissorCount = 1;
	pi->viewportState.pScissors = &pi->scissor;
	
	pi->viewport.x = 0;
	pi->viewport.y = 0;
	pi->viewport.width = rp->FBWidth;
	pi->viewport.height = rp->FBHeight;
	pi->viewport.minDepth = 0.0f;
	pi->viewport.maxDepth = 1.0f;
	
	pi->scissor.offset.x = 0;
	pi->scissor.offset.y = 0;
	pi->scissor.extent.width = rp->FBWidth;
	pi->scissor.extent.height = rp->FBHeight;
	
	pi->vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pi->vertexInputInfo.pVertexBindingDescriptions = pi->vertexBindings;
	pi->vertexInputInfo.pVertexAttributeDescriptions = pi->vertexAttributes;
	
	pi->vertexInputInfo.vertexBindingDescriptionCount = numVertexBindings;
	pi->vertexInputInfo.vertexAttributeDescriptionCount = numVertexBindings;
	
	for( int i = 0; i < 8; i++ ) {
		pi->vertexBindings[i].binding = i;
		pi->vertexBindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		pi->vertexAttributes[i].location = i;
		pi->vertexAttributes[i].binding = i;
	}
	
	pi->inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pi->inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pi->inputAssembly.primitiveRestartEnable = VK_FALSE;
	
	pi->rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pi->rasterizer.depthClampEnable = VK_FALSE;
	pi->rasterizer.rasterizerDiscardEnable = VK_FALSE;
	pi->rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	pi->rasterizer.lineWidth = 1.0f;
	pi->rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	pi->rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pi->rasterizer.depthBiasEnable = VK_FALSE;
	
	pi->multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pi->multisampling.sampleShadingEnable = VK_FALSE;
	pi->multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	
	pi->colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pi->colourBlending.logicOpEnable = VK_FALSE;
	pi->colourBlending.attachmentCount = rp->subPassAttachmentCounts[subPass];
	pi->colourBlending.pAttachments = pi->colourBlendAttachments;
	
	for( int i = 0; i < 8; i++ ) {
		pi->colourBlendAttachments[i].blendEnable = VK_FALSE;
		pi->colourBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_RGBA_BITS;
	}
	
	pi->depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pi->depthState.depthTestEnable = FALSE;
	pi->depthState.depthWriteEnable = TRUE;
	pi->depthState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
	pi->depthState.depthBoundsTestEnable = FALSE;
	pi->depthState.stencilTestEnable = FALSE;
	
	pi->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pi->pipelineLayoutCreateInfo.setLayoutCount = numDescriptorLayouts;
	pi->pipelineLayoutCreateInfo.pSetLayouts = pi->descriptorLayouts;
	pi->pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pi->pipelineLayoutCreateInfo.pPushConstantRanges = pi->pushConsts;
	
	pi->numDescriptorLayouts = numDescriptorLayouts;
	for( int i = 0; i < 8; i++ ) {
		pi->descriptorLayoutInfo[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		pi->descriptorLayoutInfo[i].bindingCount = 0;
		pi->descriptorLayoutInfo[i].pBindings = pi->descriptorLayoutBindings[i];
	}
	
	return pi;
}

void VKTR_PipelineSetShader( VKTR_Pipeline pipeline, VKTR_ShaderStage stage, VKTR_Shader shader ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	int n = pi->numStages++;
	pi->stageCreateInfo[n].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pi->stageCreateInfo[n].stage = stageMap[stage];
	pi->stageCreateInfo[n].module = shader;
	pi->stageCreateInfo[n].pName = "main";
}

void VKTR_PipelineSetDepthFunction( VKTR_Pipeline pipeline, VKTR_DepthFunction function ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	VkCompareOp depthFnMap[] = {
		VK_COMPARE_OP_LESS,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_COMPARE_OP_EQUAL,
		VK_COMPARE_OP_NOT_EQUAL,
		VK_COMPARE_OP_GREATER_OR_EQUAL,
		VK_COMPARE_OP_GREATER,
		VK_COMPARE_OP_ALWAYS,
		VK_COMPARE_OP_NEVER
	};
	
	pi->depthState.depthCompareOp = depthFnMap[function];
}

void VKTR_PipelineSetDepthEnable( VKTR_Pipeline pipeline, int testEnable, int writeEnable ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	pi->depthState.depthTestEnable = testEnable;
	pi->depthState.depthWriteEnable = writeEnable;
}

void VKTR_PipelineSetViewport( VKTR_Pipeline pipeline, float x, float y, float z, float width, float height, float depth ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	pi->viewport.x = x;
	pi->viewport.y = y;
	pi->viewport.width = width;
	pi->viewport.height = height;
	pi->viewport.minDepth = z;
	pi->viewport.maxDepth = depth;
}

void VKTR_PipelineSetScissor( VKTR_Pipeline pipeline, u32 x, u32 y, u32 width, u32 height ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	pi->scissor.offset.x = x;
	pi->scissor.offset.y = y;
	pi->scissor.extent.width = width;
	pi->scissor.extent.height = height;
}

void VKTR_PipelineSetBlend( VKTR_Pipeline pipeline, u32 attachment, VKTR_BlendOp blendOp, VKTR_BlendFactor src, VKTR_BlendFactor dst ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	VkBlendOp blendOpMap[] = {
		VK_BLEND_OP_ADD,
		VK_BLEND_OP_SUBTRACT,
		VK_BLEND_OP_REVERSE_SUBTRACT,
		VK_BLEND_OP_MIN,
		VK_BLEND_OP_MAX 
	};
	
	VkBlendFactor blendFactorMap[] = {
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_SRC_ALPHA,
		VK_BLEND_FACTOR_DST_ALPHA,
		VK_BLEND_FACTOR_SRC_COLOR,
		VK_BLEND_FACTOR_DST_COLOR,
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
		VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR
	};
	
	pi->colourBlendAttachments[attachment].colorBlendOp = blendOpMap[blendOp];
	pi->colourBlendAttachments[attachment].srcColorBlendFactor = blendFactorMap[src];
	pi->colourBlendAttachments[attachment].dstColorBlendFactor = blendFactorMap[dst];
}

void VKTR_PipelineSetBlendEnable( VKTR_Pipeline pipeline, u32 attachment, int enable ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	pi->colourBlendAttachments[attachment].blendEnable = enable;
}

void VKTR_PipelineSetVertexBinding( VKTR_Pipeline pipeline, u32 binding, u32 offset, u32 stride, VKTR_Format format ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	pi->vertexBindings[binding].stride = stride;
	
	pi->vertexAttributes[binding].offset = offset;
	pi->vertexAttributes[binding].format = formatMap[format];
}

void VKTR_PipelineSetDescriptorBinding( VKTR_Pipeline pipeline, u32 layoutIndex, u32 binding, VKTR_ShaderStage stage, VKTR_DescriptorType type, u32 count ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
    pi->descriptorLayoutBindings[layoutIndex][binding].binding = binding;
    pi->descriptorLayoutBindings[layoutIndex][binding].descriptorType = descriptorMap[type];
    pi->descriptorLayoutBindings[layoutIndex][binding].descriptorCount = count;
    pi->descriptorLayoutBindings[layoutIndex][binding].stageFlags = stageMap[stage];
    pi->descriptorLayoutBindings[layoutIndex][binding].pImmutableSamplers = NULL;
	
    if( binding >= pi->descriptorLayoutInfo[layoutIndex].bindingCount ) {
		pi->descriptorLayoutInfo[layoutIndex].bindingCount = binding + 1;
	}
}

void VKTR_PipelineSetImmutableSamplerBinding( VKTR_Pipeline pipeline, u32 layoutIndex, u32 binding, VKTR_ShaderStage stage, VKTR_Sampler sampler ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
    pi->descriptorLayoutBindings[layoutIndex][binding].binding = binding;
    pi->descriptorLayoutBindings[layoutIndex][binding].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    pi->descriptorLayoutBindings[layoutIndex][binding].descriptorCount = 1;
    pi->descriptorLayoutBindings[layoutIndex][binding].stageFlags = stageMap[stage];
    pi->descriptorLayoutBindings[layoutIndex][binding].pImmutableSamplers = (VkSampler*)&sampler;
	
    if( binding >= pi->descriptorLayoutInfo[layoutIndex].bindingCount ) {
		pi->descriptorLayoutInfo[layoutIndex].bindingCount = binding + 1;
	}
}

void VKTR_PipelineSetPushConstRange( VKTR_Pipeline pipeline, int offset, int size, VKTR_ShaderStage stage ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	int count = pi->pipelineLayoutCreateInfo.pushConstantRangeCount;
	
	pi->pushConsts[count].stageFlags = stageMap[stage];
	pi->pushConsts[count].offset = offset;
	pi->pushConsts[count].size = size;
	
	pi->pipelineLayoutCreateInfo.pushConstantRangeCount += 1;
}

void VKTR_BuildPipeline( VKTR_Pipeline pipeline ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	for( int i = 0; i < pi->numDescriptorLayouts; i++ ) {
		vkCreateDescriptorSetLayout( device, &pi->descriptorLayoutInfo[i], NULL, &pi->descriptorLayouts[i] );
	}
	
	vkCreatePipelineLayout( device, &pi->pipelineLayoutCreateInfo, NULL, &pi->pipelineLayout );
	
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = pi->numStages;
	pipelineCreateInfo.pStages = pi->stageCreateInfo;
	pipelineCreateInfo.pVertexInputState = &pi->vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &pi->inputAssembly;
	pipelineCreateInfo.pViewportState = &pi->viewportState;
	pipelineCreateInfo.pRasterizationState = &pi->rasterizer;
	pipelineCreateInfo.pMultisampleState = &pi->multisampling;
	pipelineCreateInfo.pDepthStencilState = &pi->depthState;
	pipelineCreateInfo.pColorBlendState = &pi->colourBlending;
	pipelineCreateInfo.pDynamicState = NULL;
	pipelineCreateInfo.layout = pi->pipelineLayout;
	pipelineCreateInfo.renderPass = pi->renderPass->vkRenderPass;
	pipelineCreateInfo.subpass = pi->subPass;
	
	vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &pi->pipeline );
}

VkFilter filterMap[] = {
	VK_FILTER_NEAREST,
	VK_FILTER_LINEAR
};

VkSamplerMipmapMode mipFilterMap[] = {
	VK_SAMPLER_MIPMAP_MODE_NEAREST,
	VK_SAMPLER_MIPMAP_MODE_LINEAR
};

VkSamplerAddressMode wrapMap[] = {
	VK_SAMPLER_ADDRESS_MODE_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
};

VKTR_Sampler VKTR_CreateSampler( VKTR_FilterMode min, VKTR_FilterMode mag, VKTR_FilterMode mip, VKTR_WrapMode wrap, float maxAniso ) {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.minFilter = filterMap[min];
	samplerInfo.magFilter = filterMap[mag];
	samplerInfo.mipmapMode = mipFilterMap[mip];
	samplerInfo.addressModeU = wrapMap[wrap];
	samplerInfo.addressModeV = wrapMap[wrap];
	samplerInfo.anisotropyEnable = maxAniso > 1.0f;
	samplerInfo.maxAnisotropy = maxAniso;
	
	VkSampler sampler;
	vkCreateSampler( device, &samplerInfo, NULL, &sampler );
	
	return sampler;
}

VKTR_DescriptorPool VKTR_CreateDescriptorPool( u32 descriptorCount, u32 setCount, u32 inlineUniformBytes ) {
	VkDescriptorType descriptorTypes[6] = {
		VK_DESCRIPTOR_TYPE_SAMPLER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
	};
	
	VkDescriptorPoolSize poolSize[6] = {};
	for( int i = 0; i < 5; i++ ) {
		poolSize[i].type = descriptorTypes[i];
		poolSize[i].descriptorCount = descriptorCount;
	}
	
	poolSize[5].type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
	poolSize[5].descriptorCount = inlineUniformBytes;
	
	VkDescriptorPoolInlineUniformBlockCreateInfoEXT inlineUniformCreateInfo = {};
	inlineUniformCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO_EXT;
	inlineUniformCreateInfo.maxInlineUniformBlockBindings = descriptorCount;
	
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = &inlineUniformCreateInfo;
	poolInfo.poolSizeCount = 6;
	poolInfo.pPoolSizes = poolSize;
	poolInfo.maxSets = setCount;
	
	VkDescriptorPool pool;
	vkCreateDescriptorPool( device, &poolInfo, NULL, &pool );
	
	return pool;
}

void VKTR_ResetDescriptorPool( VKTR_DescriptorPool pool ) {
	vkResetDescriptorPool( device, pool, 0 );
}

VKTR_DescriptorSet VKTR_CreateDescriptorSet( VKTR_DescriptorPool pool, VKTR_Pipeline pipeline, int layoutIndex ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &pi->descriptorLayouts[layoutIndex];
	
	VkDescriptorSet set;
	vkAllocateDescriptorSets( device, &allocInfo, &set );
	
	return set;
}

void updateDescriptorSet( VkDescriptorSet set, int binding, int element, VkDescriptorType type, void* bufferInfo, void* imageInfo, void* texelBuffer, void* pNext ) {
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = set;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = element;
	descriptorWrite.descriptorType = type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = bufferInfo;
	descriptorWrite.pImageInfo = imageInfo;
	descriptorWrite.pTexelBufferView = texelBuffer;
	
	vkUpdateDescriptorSets( device, 1, &descriptorWrite, 0, NULL );
}

void VKTR_SetTextureBinding( VKTR_DescriptorSet set, int binding, int element, VKTR_Image image ) {
	VKTR_Internal_Image* img = image;
	
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageView = img->defaultView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	
	updateDescriptorSet( set, binding, element, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, NULL, &imageInfo, NULL, NULL );
}

void VKTR_SetSamplerBinding( VKTR_DescriptorSet set, int binding, int element, VKTR_Sampler sampler ) {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.sampler = sampler;
	//imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	
	updateDescriptorSet( set, binding, element, VK_DESCRIPTOR_TYPE_SAMPLER, NULL, &imageInfo, NULL, NULL );
}

void VKTR_SetUniformBinding( VKTR_DescriptorSet set, int binding, int element, VKTR_Buffer buffer, int offset, int size ) {
	VKTR_Internal_Buffer* buf = buffer;
	
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buf->buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = size;
	
	updateDescriptorSet( set, binding, element, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo, NULL, NULL, NULL );
}

void VKTR_SetInlineUniformBinding( VKTR_DescriptorSet set, int binding, int size, void* data ) {
	VkWriteDescriptorSetInlineUniformBlockEXT inlineInfo = {};
	inlineInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT;
	inlineInfo.dataSize = size;
	inlineInfo.pData = data;
	
	updateDescriptorSet( set, binding, 0, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, NULL, NULL, NULL, &inlineInfo );
}

void VKTR_SetInputAttachmentBinding( VKTR_DescriptorSet set, int binding, int element, VKTR_Image image ) {
	VKTR_Internal_Image* img = image;
	
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageView = img->defaultView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	
	updateDescriptorSet( set, binding, element, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, NULL, &imageInfo, NULL, NULL );
}

void VKTR_CMD_BindVertexBuffer( VKTR_CommandBuffer buffer, VKTR_Buffer vertexBuffer, int binding ) {
	VKTR_Internal_Buffer* vbuf = vertexBuffer;
	
	size_t offset = 0;
	vkCmdBindVertexBuffers( buffer, binding, 1, &vbuf->buffer, &offset );
} 

void VKTR_CMD_BindIndexBuffer( VKTR_CommandBuffer buffer, VKTR_Buffer indexBuffer, VKTR_IndexType indexType ) {
	VKTR_Internal_Buffer* ibuf = indexBuffer;
	
	vkCmdBindIndexBuffer( buffer, ibuf->buffer, 0, indexMap[indexType] );
}

void VKTR_CMD_DrawIndexed( VKTR_CommandBuffer buffer, u32 count ) {
	vkCmdDrawIndexed( buffer, count, 1, 0, 0, 0 );
}

void VKTR_CMD_DrawIndexedOffset( VKTR_CommandBuffer buffer, u32 offset, u32 count, u32 vertexOffset ) {
	vkCmdDrawIndexed( buffer, count, 1, offset, vertexOffset, 0 );
}

void VKTR_CMD_Draw( VKTR_CommandBuffer buffer, u32 count ) {
	vkCmdDraw( buffer, count, 1, 0, 0 );
}

void VKTR_CMD_DrawOffset( VKTR_CommandBuffer buffer, u32 offset, u32 count ) {
	vkCmdDraw( buffer, count, 1, offset, 0 );
}

void VKTR_CMD_StartRenderPass( VKTR_CommandBuffer buffer, VKTR_RenderPass renderPass ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	VkRenderPassBeginInfo renderpassInfo = {};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassInfo.renderPass = rp->vkRenderPass;
	renderpassInfo.framebuffer = rp->vkFramebuffer;
	renderpassInfo.renderArea.offset.x = 0;
	renderpassInfo.renderArea.offset.y = 0;
	renderpassInfo.renderArea.extent.width = rp->FBWidth;
	renderpassInfo.renderArea.extent.height = rp->FBHeight;
	
	VkClearValue clearColours[8];
	memset( clearColours, 0, sizeof( clearColours ) );
	
	//clearColours[1].depthStencil.depth = 0.0f;
	
	renderpassInfo.clearValueCount = 8;
	renderpassInfo.pClearValues = clearColours;
	
	vkCmdBeginRenderPass( buffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void VKTR_CMD_NextSubPass( VKTR_CommandBuffer buffer ) {
	vkCmdNextSubpass( buffer, VK_SUBPASS_CONTENTS_INLINE );
}

void VKTR_CMD_EndRenderPass( VKTR_CommandBuffer buffer ) {
	vkCmdEndRenderPass( buffer );
}

void VKTR_CMD_Clear( VKTR_CommandBuffer buffer, u32 attachment, vec4 clearVal ) {
	VkClearAttachment atc = {};
	atc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	atc.colorAttachment = attachment;
	atc.clearValue.color.float32[0] = clearVal[0];
	atc.clearValue.color.float32[1] = clearVal[1];
	atc.clearValue.color.float32[2] = clearVal[2];
	atc.clearValue.color.float32[3] = clearVal[3];
	
	VkClearRect cr = {};
	cr.rect.offset.x = 0;
	cr.rect.offset.y = 0;
	cr.rect.extent.width = INT_MAX; // FIX
	cr.rect.extent.height = INT_MAX; // FIX
	cr.baseArrayLayer = 0;
	cr.layerCount = 1;
	
	vkCmdClearAttachments( buffer, 1, &atc, 1, &cr );
}

void VKTR_CMD_CopyToImage( VKTR_CommandBuffer buffer, VKTR_Buffer src, VKTR_Image dst ) {
	VKTR_Internal_Buffer* buf = src;
	VKTR_Internal_Image* img = dst;
	
	VkImageMemoryBarrier layoutBarrier = {};
	layoutBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	layoutBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	layoutBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	layoutBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//layoutBarrier.srcQueueFamilyIndex = transferQueueFamily;
	//layoutBarrier.dstQueueFamilyIndex = transferQueueFamily;
	layoutBarrier.srcQueueFamilyIndex = graphicsQueueFamily;
	layoutBarrier.dstQueueFamilyIndex = graphicsQueueFamily;
	layoutBarrier.image = img->tex;
	layoutBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layoutBarrier.subresourceRange.baseMipLevel = 0;
	layoutBarrier.subresourceRange.levelCount = 1;
	layoutBarrier.subresourceRange.baseArrayLayer = 0;
	layoutBarrier.subresourceRange.layerCount = 1;
	
	vkCmdPipelineBarrier( buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &layoutBarrier );
	
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = img->w;
	region.bufferImageHeight = img->h;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // FIX
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageOffset.z = 0;
	region.imageExtent.width = img->w;
	region.imageExtent.height = img->h;
	region.imageExtent.depth = 1;
	
	vkCmdCopyBufferToImage( buffer, buf->buffer, img->tex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
	
	VkImageMemoryBarrier layoutBarrier2 = {};
	layoutBarrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutBarrier2.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	layoutBarrier2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	layoutBarrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	layoutBarrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//layoutBarrier2.srcQueueFamilyIndex = transferQueueFamily;
	//layoutBarrier2.dstQueueFamilyIndex = transferQueueFamily;
	layoutBarrier2.srcQueueFamilyIndex = graphicsQueueFamily;
	layoutBarrier2.dstQueueFamilyIndex = graphicsQueueFamily;
	layoutBarrier2.image = img->tex;
	layoutBarrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layoutBarrier2.subresourceRange.baseMipLevel = 0;
	layoutBarrier2.subresourceRange.levelCount = 1;
	layoutBarrier2.subresourceRange.baseArrayLayer = 0;
	layoutBarrier2.subresourceRange.layerCount = 1;
	
	vkCmdPipelineBarrier( buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &layoutBarrier2 );
}

VKTR_Internal_Pipeline* currentPipeline;

void VKTR_CMD_BindPipeline( VKTR_CommandBuffer buffer, VKTR_Pipeline pipeline ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
	vkCmdBindPipeline( buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pi->pipeline );
	
	currentPipeline = pi;
}

void VKTR_CMD_PushConst( VKTR_CommandBuffer buffer, VKTR_ShaderStage stage, u32 offset, u32 size, void* data ) {
	vkCmdPushConstants( buffer, currentPipeline->pipelineLayout, stageMap[stage], offset, size, data );
}

void VKTR_CMD_BindDescriptorSet( VKTR_CommandBuffer buffer, VKTR_DescriptorSet set ) {
	vkCmdBindDescriptorSets( buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->pipelineLayout, 0, 1, (VkDescriptorSet*)&set, 0, NULL );
}
