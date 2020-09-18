#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
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

// TODO replace malloc

typedef struct {
	HWND osHandle;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	u32 numSwapchainImages;
	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	u32 imageIndex;
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
} VKTR_Internal_Image;


VkInstance instance;
VkDevice device;
VkPhysicalDevice physicalDevice;

int graphicsQueueFamily = -1;
int computeQueueFamily = -1;
int transferQueueFamily = -1;

VkCommandPool commandPools[3];


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
	const char* deviceExtensions[1] = {
		"VK_KHR_swapchain",
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
	deviceCreateInfo.enabledExtensionCount = 1;
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
	appInfo.apiVersion = VK_API_VERSION_1_1;
	
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
	
	
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = graphicsQueueFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ;
	
	vkCreateCommandPool( device, &poolInfo, NULL, &commandPools[VKTR_QUEUETYPE_GRAPHICS] );
	
	poolInfo.queueFamilyIndex = computeQueueFamily >= 0 ? computeQueueFamily : graphicsQueueFamily;
	vkCreateCommandPool( device, &poolInfo, NULL, &commandPools[VKTR_QUEUETYPE_COMPUTE] );
	
	poolInfo.queueFamilyIndex = transferQueueFamily >= 0 ? transferQueueFamily : graphicsQueueFamily;
	vkCreateCommandPool( device, &poolInfo, NULL, &commandPools[VKTR_QUEUETYPE_TRANSFER] );
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

void createImageView( VkDevice device, VkImageView* view, VkImage image, VkFormat format ) {
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
	
	RECT bounds = { 0, 0, width, height };
	AdjustWindowRect( &bounds, WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPEDWINDOW, FALSE );
	printf( "adjusted window bounds: %d, %d, %d, %d\n", bounds.left, bounds.top, bounds.right, bounds.bottom );
	
	HWND hWnd = CreateWindow( title, title, WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, bounds.right - bounds.left, bounds.bottom - bounds.top, NULL, NULL, hInst, NULL );
	
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
	VkPresentModeKHR presentMode = flags & VKTR_WINDOWFLAG_VSYNC ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
	
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
		createImageView( device, &swapchainImageViews[i], swapchainImages[i], VK_FORMAT_R8G8B8A8_SRGB );
	}
	
	VKTR_Internal_Window* window = malloc( sizeof( VKTR_Internal_Window ) );
	window->osHandle = hWnd;
	window->surface = surface;
	window->swapchain = swapchain;
	window->numSwapchainImages = numSwapchainImages;
	window->swapchainImages = swapchainImages;
	window->swapchainImageViews = swapchainImageViews;
	
	/*
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	vkCreateSemaphore( device, &semaphoreInfo, NULL, &window->imageAvailable );
	vkCreateSemaphore( device, &semaphoreInfo, NULL, &window->renderFinished );
	vkAcquireNextImageKHR( device, swapchain, UINT64_MAX, window->imageAvailable, VK_NULL_HANDLE, &window->imageIndex );
	*/
	
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
}

/*
VKTR_Image VKTR_GetWindowSurface( VKTR_Window window ) {
	VKTR_Internal_Window* win = window;
	
	return win->swapchainImages[win->imageIndex];
}
*/

VKTR_Image VKTR_GetSwapchainImage( VKTR_Window window, u32 n ) {
	VKTR_Internal_Window* win = window;
	
	VKTR_Internal_Image* img = malloc( sizeof( VKTR_Internal_Image ) );
	img->tex = win->swapchainImages[n];
	
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
	
	if( type == VKTR_FORMAT_D32F ) {
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	} else {
		imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	
	if( flags | VKTR_IMAGEFLAG_SUBPASSINPUT ) {
		imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
	
	vkCreateImage( device, &imageInfo, NULL, &img->tex );
	
	VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements( device, img->tex, &memRequirements );

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = 2;

	vkAllocateMemory( device, &allocInfo, NULL, &img->texMemory );

    vkBindImageMemory( device, img->tex, img->texMemory, 0 );
    
    img->memorySize = allocInfo.allocationSize;
    
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

VKTR_CommandBuffer VKTR_CreateCommandBuffer( VKTR_QueueType queueType ) {
	VkCommandBuffer cmdBuf;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPools[queueType];
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers( device, &allocInfo, &cmdBuf );
	
	return cmdBuf;
}

void VKTR_StartRecording( VKTR_CommandBuffer buffer ) {
	vkResetCommandBuffer( buffer, 0 );
	
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		
	vkBeginCommandBuffer( buffer, &beginInfo );
}

void VKTR_EndRecording( VKTR_CommandBuffer buffer ) {
	vkEndCommandBuffer( buffer );
}

void VKTR_Submit( VKTR_Queue queue, VKTR_CommandBuffer buffer, VKTR_Semaphore waitSemaphore, VKTR_Semaphore signalSemaphore ) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = (VkSemaphore*)&waitSemaphore;
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = (VkCommandBuffer*)&buffer;
	submitInfo.signalSemaphoreCount = 1;
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
	
	return renderPass;
}

void VKTR_SetRenderPassAttachment( VKTR_RenderPass renderPass, u32 attachment, VKTR_Image image, VKTR_Format format, int loadInput, int storeOutput ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	VKTR_Internal_Image* img = image;
	
	memset( &rp->attachments[attachment], 0, sizeof( VkAttachmentDescription ) );
	
	VkAttachmentDescription* at = &rp->attachments[attachment];
	at->format = formatMap[format];
	at->samples = VK_SAMPLE_COUNT_1_BIT; // FIX
	at->loadOp = loadInput ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR; // FIX
	at->storeOp = storeOutput ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	at->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	at->initialLayout = loadInput ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_UNDEFINED; // FIX
	at->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // FIX
	
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = img->tex;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = at->format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask = ( format == VKTR_FORMAT_D32F ) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	vkCreateImageView( device, &viewCreateInfo, NULL, &rp->vkFBImageViews[attachment] );
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
	
	VkSubpassDependency subPassDependencies[8];
	int numDependencies = 0;
	
	for( int i = 0; i < 8; i++ ) {
		subPasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPasses[i].pColorAttachments = attachmentRefs[i];
		subPasses[i].pInputAttachments = inputAttachmentRefs[i];
		
		subPasses[i].pDepthStencilAttachment = &depthAttachmentRefs[i];
		depthAttachmentRefs[i].attachment = rp->subPassDepth[i];
		depthAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		int numAttachments = 0;
		int numInputAttachments = 0;
		
		for( int j = 0; j < 8; j++ ) {
			attachmentRefs[i][j].attachment = VK_ATTACHMENT_UNUSED;
			attachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_UNDEFINED;
			
			if( rp->subPassInputs[i][j] != VK_ATTACHMENT_UNUSED ) {
				inputAttachmentRefs[i][j].attachment = rp->subPassInputs[i][j];
				//inputAttachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				inputAttachmentRefs[i][j].layout = VK_IMAGE_LAYOUT_GENERAL;
				
				if( j < 0 ) {
					for( int k = j - 1; k >= 0; k++ ) {
						for( int l = 0; l < subPasses[k].colorAttachmentCount; l++ ) {
							if( rp->subPassInputs[i][j] == rp->subPassOutputs[k][l] ) {
								subPassDependencies[numDependencies].srcSubpass = k;
								subPassDependencies[numDependencies].dstSubpass = i;
								subPassDependencies[numDependencies].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
								subPassDependencies[numDependencies].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
								subPassDependencies[numDependencies].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
								subPassDependencies[numDependencies].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
								subPassDependencies[numDependencies].dependencyFlags = 0; // VK_DEPENDENCY_BY_REGION_BIT;
								numDependencies++;
							}
						}
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
			}
		}
		
		subPasses[i].colorAttachmentCount = numAttachments;
		rp->subPassAttachmentCounts[i] = numAttachments;
		
		subPasses[i].inputAttachmentCount = numInputAttachments;
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
	framebufferInfo.width = 1280;//width;
	framebufferInfo.height = 720;//height;
	framebufferInfo.layers = 1;
	
	vkCreateFramebuffer( device, &framebufferInfo, NULL, &rp->vkFramebuffer );
}
/*
void VKTR_FreeRenderPass( VKTR_RenderPass renderPass ) {
	
}
*/

VKTR_Pipeline VKTR_CreatePipeline( VKTR_RenderPass renderPass, u32 subPass, int numDescriptorLayouts ) {
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
	
	pi->vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pi->vertexInputInfo.pVertexBindingDescriptions = pi->vertexBindings;
	pi->vertexInputInfo.pVertexAttributeDescriptions = pi->vertexAttributes;
	
	pi->vertexInputInfo.vertexBindingDescriptionCount = 8; // FIX
	pi->vertexInputInfo.vertexAttributeDescriptionCount = 8; // FIX
	
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
	pi->pipelineLayoutCreateInfo.pSetLayouts = pi->descriptorLayoutInfo;
	pi->pipelineLayoutCreateInfo.pushConstantRangeCount = 2;
	pi->pipelineLayoutCreateInfo.pPushConstantRanges = pi->pushConsts;
	
	pi->numDescriptorLayouts = numDescriptorLayouts;
	for( int i = 0; i < 8; i++ ) {
		pi->descriptorLayoutInfo[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		pi->descriptorLayoutInfo[i].bindingCount = 0;
		pi->descriptorLayoutInfo[i].pBindings = pi->descriptorLayoutBindings[i];
	}
    
	// FIX
	pi->pushConsts[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pi->pushConsts[0].offset = 0;
	pi->pushConsts[0].size = 16 * 4;
	
	pi->pushConsts[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pi->pushConsts[1].offset = 64;
	pi->pushConsts[1].size = 16;
	
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
	
    if( binding > pi->descriptorLayoutInfo[layoutIndex].bindingCount ) {
		pi->descriptorLayoutInfo[layoutIndex].bindingCount = binding;
	}
}

void VKTR_BuildPipeline( VKTR_Pipeline pipeline ) {
	VKTR_Internal_Pipeline* pi = pipeline;
	
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

void VKTR_CreateDescriptorSet( void ) {
	/*
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;
	
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;
	
	VkDescriptorSetLayout descriptorSetLayout;
    vkCreateDescriptorSetLayout( device, &layoutInfo, NULL, &descriptorSetLayout );
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 8;
	
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 8;
	
	VkDescriptorPool descriptorPool;
	vkCreateDescriptorPool( device, &poolInfo, NULL, &descriptorPool );
	
	VkDescriptorSetLayout layouts[8];
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 8;
	allocInfo.pSetLayouts = layouts;
	
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSets[8];
	
	vkAllocateDescriptorSets( device, &allocInfo, descriptorSets );
	
	VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffers[0];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof( UniformBufferObject );
    
    VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSets[0];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = NULL;
	descriptorWrite.pTexelBufferView = NULL;
	
	vkUpdateDescriptorSets( device, 1, &descriptorWrite, 0, NULL );
	
	vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[0], 0, NULL );
	*/
}

void VKTR_CMD_BindVertexBuffer( VKTR_CommandBuffer buffer, VKTR_Buffer vertexBuffer, int binding ) {
	VKTR_Internal_Buffer* vbuf = vertexBuffer;
	
	size_t offset = 0;
	vkCmdBindVertexBuffers( buffer, binding, 1, &vbuf->buffer, &offset );
} 

//void VKTR_CMD_Draw( VKTR_CommandBuffer buffer, VKTR_Buffer indexBuffer, VKTR_IndexType indexType, VKTR_Buffer vertexBuffer, u32 count ) {
void VKTR_CMD_DrawIndexed( VKTR_CommandBuffer buffer, VKTR_Buffer indexBuffer, VKTR_IndexType indexType, u32 count ) {
	//VKTR_Internal_Buffer* vbuf = vertexBuffer;
	VKTR_Internal_Buffer* ibuf = indexBuffer;
	
	//vkCmdBindVertexBuffers( buffer, 1, 1, &vbuf->buffer, &offset );
	vkCmdBindIndexBuffer( buffer, ibuf->buffer, 0, VK_INDEX_TYPE_UINT32 );
	vkCmdDrawIndexed( buffer, count, 1, 0, 0, 0 );
}

void VKTR_CMD_Draw( VKTR_CommandBuffer buffer, u32 count ) {
	vkCmdDraw( buffer, count, 1, 0, 0 );
}

void VKTR_CMD_StartRenderPass( VKTR_CommandBuffer buffer, VKTR_RenderPass renderPass ) {
	VKTR_Internal_RenderPass* rp = renderPass;
	
	VkRenderPassBeginInfo renderpassInfo = {};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassInfo.renderPass = rp->vkRenderPass;
	renderpassInfo.framebuffer = rp->vkFramebuffer;
	renderpassInfo.renderArea.offset.x = 0;
	renderpassInfo.renderArea.offset.y = 0;
	renderpassInfo.renderArea.extent.width = 1280;//width;
	renderpassInfo.renderArea.extent.height = 720;//height;
	
	VkClearValue clearColours[8];
	memset( clearColours, 0, sizeof( clearColours ) );
	
	clearColours[1].depthStencil.depth = 1.0f;
	
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
	cr.rect.extent.width = 1280; // FIX
	cr.rect.extent.height = 720; // FIX
	cr.baseArrayLayer = 0;
	cr.layerCount = 1;
	
	vkCmdClearAttachments( buffer, 1, &atc, 1, &cr );
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
