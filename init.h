#pragma once

/*

Header contains both declarations and definitions.

Include header in .c file for definitions:
#define VKCMDINIT_IMPL
#include <init.h>
For declarations don't provide VKCMDINIT_IMPL, so you don't get
symbol redeclarations:
#include <init.h>

Download Vulkan SDK from lunarg (https://www.lunarg.com/vulkan-sdk/).
Library uses CRT for memory allocation.

Defines:

#define VKCMDINIT_GLFW - If you use GLFW for crossplatform windows (recommended)
#define VKCMDINIT_INCLUDED_VULKAN - If vulkan.h is already included
#define VKCMDINIT_IMPL - includes definitions (function bodies)

*/


#ifndef VKCMDINIT_INCLUDED_VULKAN
#include <vulkan/vulkan.h>
#else
#ifndef VULKAN_H_
#error "Vulkan is missing, but VKCMDINIT_INCLUDED_VULKAN is specified"
#endif
#endif

#ifdef VKCMDINIT_GLFW
#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#endif

#include <stdbool.h>

//DO NOT use if you specified custom deviceDesigner in createDevice
typedef struct DefaultQueueRetrieveStruct
{
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
} DefaultQueueRetrieveStruct;

//DO NOT use if you specified custom deviceDesigner in createDevice
typedef struct DefaultQueueIndices
{
	uint32_t graphicQueueIndex;
	uint32_t presentationFamilyIndex;
} DefaultQueueIndices;

//Struct containing all initialization data
typedef struct InitializationStruct
{
	union
	{
		struct EarlyInit
		{
			VkApplicationInfo appInfo;
			VkInstanceCreateInfo instanceInfo;
			size_t extensionCount;
			const char** extensionPtr;
		};
	};

	union
	{
		void* queueIndices;
		DefaultQueueIndices* defaultQueueIndices;
	};

	struct InstanceOptional
	{
		enum InstanceOptionalFlags
		{
			INSTANCE_OPTIONAL_FLAGS_DEBUG_MESSENGER = 1,
			INSTANCE_OPTIONAL_FLAGS_SURFACE = 2
		} instanceOptionalFlags;

		struct DebugOutput
		{
			VkDebugUtilsMessengerEXT debugMessenger;
		};

		struct Surface
		{
			VkSurfaceKHR surface;
		};
	};

	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

} InitializationStruct;

//Creates instance of InitialziationStruct with basic application information
InitializationStruct createApplication(
	const char* engineName,
	const char* applicationName,
	uint32_t applicationVersion,
	uint32_t engineVersion,
	uint32_t apiVersion
);

//Adds instance extension, for device extensions see createDevice
InitializationStruct addExtension(
	InitializationStruct initStruct,
	const char* extensionName
);

//Starts a vulkan instance with extensions provided using addExtension
InitializationStruct startInstance(
	InitializationStruct  initStruct
);

//Terminates vulkan instance, aswell as cleans up instance extension objects created using vkCmdInit
void terminateInstance(
	InitializationStruct initStruct
);

//Enumerates and selects a suitable physical device, provide deviceEnumerator for custom selection rules
InitializationStruct selectPhysicalDevices(
	InitializationStruct initStruct, 
	/*can be NULL. If so, selects first device available*/ VkPhysicalDevice(*deviceEnumerator)(const VkPhysicalDevice* const devices, 
	size_t deviceCount, /*ex. required device extensions*/ void* additionalData), 
	/*if device enumerator is null, can be null, because default implementation ignores it*/ void* additionalData
);

//Creates logical device with extensions provided
InitializationStruct createDevice(
	InitializationStruct initStruct, 
	/*can be NULL. If so, selects first device available, returns data that will be saved in queueIndices*/ void* (*deviceDesigner)(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* deviceCreateInfo, const char* const* deviceExtensions, size_t deviceExtensionCount), 
	const char* const* deviceExtensions, 
	size_t deviceExtensionCount
);

//retrieves queues from physical device object, if deviceDesigner in createDevice call wasn't NULL, provide custom queueRetriever function
void retrieveQueues(
	InitializationStruct initializationStruct, 
	/*if queueRetriever isn't custom, should be array of a graphics queue and a presentation queue (non-initialized if VkSurfaceKHR isn't present)*/ VkQueue* queues, 
	/*can be null if createDevice was called with deviceDesigner = null, otherwise you need to supply own retriewer*/ void(*queueRetriever)(void* queueIndices, VkDevice device, VkQueue* queues)
);

//Creates debug messenger, for simpler calls see withDebugOutput1 and withDebugOutput2
InitializationStruct withDebugOutput(
	InitializationStruct initStruct, 
	PFN_vkDebugUtilsMessengerCallbackEXT debugOutputProc, 
	void* userData, 
	VkDebugUtilsMessageSeverityFlagsEXT messageSeverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageType
);

//Creates debug messenger with default settings and pointer to user data, for call without userData see withDebugOutput2
InitializationStruct withDebugOutput1(
	InitializationStruct initStruct, 
	PFN_vkDebugUtilsMessengerCallbackEXT debugOutputProc, 
	void* userData
);

//Creates debug messenger with default settings and no user data
InitializationStruct withDebugOutput2(
	InitializationStruct initStruct, 
	PFN_vkDebugUtilsMessengerCallbackEXT debugOutputProc
);

#ifdef VKCMDINIT_GLFW
//Creates window surface using GLFWwindow (platform-independent)
InitializationStruct withSurfaceGLFW(
	InitializationStruct initStruct, 
	GLFWwindow* window
);
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
//Creates window surface for Win32 platform-dependent window
InitializationStruct withSurfaceWin32(
	InitializationStruct initStruct, 
	HINSTANCE processHandle, 
	HWND windowHandle
);
#endif

//Creates generic swapchain and retrieves images and image views from it
VkSwapchainKHR createSwapchainKHR(
	InitializationStruct initStruct, 
	/*can be null, if so, uses default selector*/ VkSurfaceFormatKHR(*surfaceFormatSelector)(VkSurfaceFormatKHR* surfaceFormats, size_t surfaceFormatCount), 
	/*can be null, if so, uses default selector*/ VkPresentModeKHR(*presentModeSelector)(VkPresentModeKHR* presentModes, size_t presentModeCount), 
	/*cannot be null*/ void(*surfaceDesigner)(const VkSurfaceCapabilitiesKHR* capabilities, VkExtent2D* extent, uint32_t* imageCount), 
	uint32_t* swapchainImageCount, 
	VkImage** swapchainImages, 
	/*can be null*/ VkImageView** swapchainImageViews
);

#ifdef VKCMDINIT_IMPL
#include <stdlib.h>

InitializationStruct createApplication(
	const char* engineName,
	const char* applicationName,
	uint32_t applicationVersion,
	uint32_t engineVersion,
	uint32_t apiVersion
)
{
	InitializationStruct initStruct = {0};

	VkApplicationInfo appInfo = {0};
	
	appInfo.engineVersion = engineVersion;
	appInfo.apiVersion = apiVersion;
	appInfo.applicationVersion = applicationVersion;
	appInfo.pEngineName = engineName;
	appInfo.pApplicationName = applicationName;

	appInfo.pNext = NULL;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

	VkInstanceCreateInfo instanceCreateInfo = {0};

	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	initStruct.appInfo = appInfo;
	initStruct.instanceInfo = instanceCreateInfo;

	initStruct.extensionCount = 0;
	initStruct.extensionPtr = NULL;
	return initStruct;
}

InitializationStruct addExtension(InitializationStruct initStruct, const char* extensionName)
{
	
	const char** ptr = (const char**)malloc(sizeof(const char* const) * (initStruct.extensionCount + 1) );
	
	for (int i = 0; i < initStruct.extensionCount; ++i)
		ptr[i] = initStruct.extensionPtr[i];


	ptr[initStruct.extensionCount] = extensionName;

	initStruct.extensionCount += 1;

	free(initStruct.extensionPtr);

	initStruct.extensionPtr = ptr;

	return initStruct;
}



InitializationStruct startInstance(InitializationStruct initStruct)
{	
	const char* validation_layers[1] =
	{
		"VK_LAYER_KHRONOS_validation"
	};
	initStruct.instanceInfo.enabledLayerCount = 1;
	initStruct.instanceInfo.ppEnabledLayerNames = validation_layers;
	initStruct.instanceInfo.pApplicationInfo = &initStruct.appInfo;
	initStruct.instanceInfo.ppEnabledExtensionNames = initStruct.extensionPtr;
	initStruct.instanceInfo.enabledExtensionCount = initStruct.extensionCount;
	VkResult r = vkCreateInstance(&initStruct.instanceInfo, NULL, &initStruct.instance);
	free(initStruct.extensionPtr);
	return initStruct;
}

void terminateInstance(InitializationStruct initStruct)
{

	if (initStruct.instanceOptionalFlags & INSTANCE_OPTIONAL_FLAGS_SURFACE)
	{
		vkDestroySurfaceKHR(initStruct.instance,initStruct.surface,NULL);
	}

	if (initStruct.instanceOptionalFlags & INSTANCE_OPTIONAL_FLAGS_DEBUG_MESSENGER)
	{
		//TODO: Store these functions
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(initStruct.instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(initStruct.instance, initStruct.debugMessenger, NULL);
	}

	vkDestroyDevice(initStruct.device,NULL);
	vkDestroyInstance(initStruct.instance, NULL);
}

InitializationStruct selectPhysicalDevices(InitializationStruct initStruct, /*can be NULL. If so, selects first device available*/ VkPhysicalDevice(*deviceEnumerator)(const VkPhysicalDevice* const devices, size_t deviceCount, /*ex. required device extensions*/ void* additionalData), /*if device enumerator is null, can be null, because default implementation ignores it*/ void* additionalData)
{
	uint32_t deviceCount;
	VkResult r = vkEnumeratePhysicalDevices(initStruct.instance, &deviceCount,NULL);
	VkPhysicalDevice* devicesAvailable = malloc(deviceCount * sizeof(VkPhysicalDevice));
	r = vkEnumeratePhysicalDevices(initStruct.instance, &deviceCount, devicesAvailable);

	if (deviceEnumerator)
	{
		initStruct.physicalDevice = deviceEnumerator(devicesAvailable, deviceCount, additionalData);
	}
	else
	{
		initStruct.physicalDevice = devicesAvailable[0];
	}

	free(devicesAvailable);

	return initStruct;
}

InitializationStruct createDevice(InitializationStruct initStruct, /*can be NULL. If so, selects first device available, returns data that will be saved in queueIndices*/ void*(*deviceDesigner)(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* deviceCreateInfo, const char* const* deviceExtensions, size_t deviceExtensionCount), const char * const * deviceExtensions, size_t deviceExtensionCount)
{
	VkDeviceCreateInfo deviceCreateInfo = {0};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	if (deviceDesigner)
	{
		initStruct.queueIndices = deviceDesigner(initStruct.physicalDevice, &deviceCreateInfo, deviceExtensions,deviceExtensionCount);
	}
	else
	{

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(initStruct.physicalDevice, &deviceFeatures);
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(initStruct.physicalDevice, &queueFamilyCount,NULL);
		VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);

		uint32_t graphicQueueIndex = 0;
		uint32_t presentationQueueIndex = 0;

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			//Do NOT use else if, there's a possibility that both queue families have same index (i.e. there's a universal queue family for 2)

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicQueueIndex = i;
			}

			if (initStruct.instanceOptionalFlags & INSTANCE_OPTIONAL_FLAGS_SURFACE)
			{
				VkBool32 supported = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(initStruct.physicalDevice,i,initStruct.surface,&supported);
				if (supported)
					presentationQueueIndex = i;
			}
		}

		free(queueFamilies);

		const float priority = 1.0f;

		VkDeviceQueueCreateInfo graphicQueueCreateInfo = {0};
		graphicQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicQueueCreateInfo.pQueuePriorities = &priority;
		graphicQueueCreateInfo.queueCount = 1;
		graphicQueueCreateInfo.queueFamilyIndex = graphicQueueIndex;
		
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &graphicQueueCreateInfo;
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
		deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
	

		DefaultQueueIndices defaultQueueIndices;
		
		defaultQueueIndices.graphicQueueIndex = graphicQueueIndex;
		defaultQueueIndices.presentationFamilyIndex = presentationQueueIndex;

		initStruct.defaultQueueIndices = malloc(sizeof(defaultQueueIndices));
		*initStruct.defaultQueueIndices = defaultQueueIndices;

	}
	vkCreateDevice(initStruct.physicalDevice, &deviceCreateInfo, NULL, &initStruct.device);
	return initStruct;
}

void retrieveQueues(InitializationStruct initializationStruct, /*if queueRetriever isn't custom, should be array of a graphics queue and a presentation queue (non-initialized if VkSurfaceKHR isn't present)*/ VkQueue* queues, /*can be null if createDevice was called with deviceDesigner = null, otherwise you need to supply own retriewer*/ void(*queueRetriever)(void* queueIndices, VkDevice device, VkQueue* queues))
{
	if (queueRetriever)
	{
		queueRetriever(initializationStruct.queueIndices,initializationStruct.device,queues);
	}
	else
	{
		vkGetDeviceQueue(initializationStruct.device,initializationStruct.defaultQueueIndices->graphicQueueIndex,0,&queues[0]);
		if (initializationStruct.instanceOptionalFlags & INSTANCE_OPTIONAL_FLAGS_SURFACE)
			vkGetDeviceQueue(initializationStruct.device, initializationStruct.defaultQueueIndices->presentationFamilyIndex, 0, &queues[1]);
	}
}

InitializationStruct withDebugOutput(InitializationStruct initStruct, PFN_vkDebugUtilsMessengerCallbackEXT debugOutputProc, void* userData, VkDebugUtilsMessageSeverityFlagsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType)
{
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = { 0 };
	debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfo.messageSeverity = messageSeverity;
	debugUtilsMessengerCreateInfo.messageType = messageType;
	debugUtilsMessengerCreateInfo.pfnUserCallback = debugOutputProc;
	debugUtilsMessengerCreateInfo.pUserData = userData;


	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(initStruct.instance, "vkCreateDebugUtilsMessengerEXT");
	vkCreateDebugUtilsMessengerEXT(initStruct.instance, &debugUtilsMessengerCreateInfo, NULL, &initStruct.debugMessenger);

	initStruct.instanceOptionalFlags |= INSTANCE_OPTIONAL_FLAGS_DEBUG_MESSENGER;

	return initStruct;
}

InitializationStruct withDebugOutput1(InitializationStruct initStruct, PFN_vkDebugUtilsMessengerCallbackEXT debugOutputProc, void* userData)
{
	//Default options from vulkan-tutorial.com
	//Thanks!
	return withDebugOutput(initStruct, debugOutputProc, userData, VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
}

InitializationStruct withDebugOutput2(InitializationStruct initStruct, PFN_vkDebugUtilsMessengerCallbackEXT debugOutputProc)
{
	return withDebugOutput1(initStruct, debugOutputProc, NULL);
}

#ifdef VKCMDINIT_GLFW
InitializationStruct withSurfaceGLFW(InitializationStruct initStruct, GLFWwindow* window)
{
	initStruct.instanceOptionalFlags |= INSTANCE_OPTIONAL_FLAGS_SURFACE;
	glfwCreateWindowSurface(initStruct.instance,window,NULL,&initStruct.surface);
	return initStruct;
}
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
//FIXME: Untested
InitializationStruct withSurfaceWin32(InitializationStruct initStruct, HINSTANCE processHandle, HWND windowHandle)
{
	initStruct.instanceOptionalFlags |= INSTANCE_OPTIONAL_FLAGS_SURFACE;

	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = { 0 };
	win32SurfaceCreateInfo.hinstance = processHandle;
	win32SurfaceCreateInfo.hwnd = windowHandle;
	win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkCreateWin32SurfaceKHR(initStruct.physicalDevice,&win32SurfaceCreateInfo,NULL,&initStruct.surface);
	return initStruct;
}

//TODO: Add Linux and Mac OS support

#endif

//PROVIDES UNIVERSAL, DEFAULT WAY OF CREATING A SWAPCHAIN
//IF YOU NEED SOMETHING MORE CUSTOM, JUST WRITE YOUR OWN FUNCTION
VkSwapchainKHR createSwapchainKHR(InitializationStruct initStruct, /*can be null, if so, uses default selector*/ VkSurfaceFormatKHR(*surfaceFormatSelector)(VkSurfaceFormatKHR* surfaceFormats, size_t surfaceFormatCount), /*can be null, if so, uses default selector*/ VkPresentModeKHR(*presentModeSelector)(VkPresentModeKHR* presentModes, size_t presentModeCount), /*cannot be null*/ void(*surfaceDesigner)(const VkSurfaceCapabilitiesKHR* capabilities, VkExtent2D* extent, uint32_t* imageCount), uint32_t* swapchainImageCount, VkImage** swapchainImages, /*can be null*/ VkImageView** swapchainImageViews)
{
	if (initStruct.instanceOptionalFlags & INSTANCE_OPTIONAL_FLAGS_SURFACE)
	{
		//Thanks vulkan-tutorial.com for that piece of code

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(initStruct.physicalDevice, initStruct.surface, &surfaceCapabilities);

		uint32_t surfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(initStruct.physicalDevice, initStruct.surface, &surfaceFormatCount, NULL);
		VkSurfaceFormatKHR* surfaceFormats = malloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(initStruct.physicalDevice, initStruct.surface, &surfaceFormatCount, surfaceFormats);

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(initStruct.physicalDevice, initStruct.surface, &presentModeCount, NULL);
		VkPresentModeKHR* presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(initStruct.physicalDevice, initStruct.surface, &presentModeCount, presentModes);

		if (presentModeCount > 0 && surfaceFormatCount > 0)
		{
			VkSurfaceFormatKHR chosenFormat;
			VkPresentModeKHR chosenMode;

			if (surfaceFormatSelector)
				chosenFormat = surfaceFormatSelector(surfaceFormats, surfaceFormatCount);
			else
			{
				chosenFormat = surfaceFormats[0];
				for (size_t i = 0; i < surfaceFormatCount; ++i)
				{
					VkSurfaceFormatKHR format = surfaceFormats[i];
					if (format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR /*preferred*/)
					{
						if (format.format == VK_FORMAT_R8G8B8_UNORM || format.format == VK_FORMAT_R8G8B8_SRGB || format.format == VK_FORMAT_R8G8B8_SNORM)
						{
							chosenFormat = format;
							break;
						}
					}
				}
			}

			if (presentModeSelector)
			{
				chosenMode = presentModeSelector(presentModes,presentModeCount);
			}
			else
			{
				for (size_t i = 0; i < presentModeCount; ++i)
				{
					VkPresentModeKHR mode = presentModes[i];
					if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR || mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
						; //ignore!
					else
					{
						//MAILBOX or FIFO will do!
						chosenMode = mode;
						break;
					}
				}
			}

			VkExtent2D extent = {0,0};
			uint32_t imageCount = 0;
			surfaceDesigner(&surfaceCapabilities,&extent,&imageCount);

			VkSwapchainCreateInfoKHR swapchainCreateinfo = {0};
			swapchainCreateinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCreateinfo.clipped = VK_TRUE;
			swapchainCreateinfo.minImageCount = imageCount;
			swapchainCreateinfo.imageExtent = extent;
			swapchainCreateinfo.surface = initStruct.surface;
			swapchainCreateinfo.imageColorSpace = chosenFormat.colorSpace;
			swapchainCreateinfo.imageFormat = chosenFormat.format;
			swapchainCreateinfo.presentMode = chosenMode;
			swapchainCreateinfo.imageArrayLayers = 1; //It's some strange thing so I won't touch it..
			swapchainCreateinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			//Once again, thanks vulkan-tutorial for this code piece
			if (initStruct.defaultQueueIndices->graphicQueueIndex != initStruct.defaultQueueIndices->presentationFamilyIndex)
			{
				swapchainCreateinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapchainCreateinfo.queueFamilyIndexCount = 2;
				swapchainCreateinfo.pQueueFamilyIndices = initStruct.defaultQueueIndices;
			}
			else
			{
				swapchainCreateinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				swapchainCreateinfo.queueFamilyIndexCount = 0;
				swapchainCreateinfo.pQueueFamilyIndices = NULL;
			}
			//TODO: Make this customizable
			swapchainCreateinfo.preTransform = surfaceCapabilities.currentTransform;
			swapchainCreateinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			
			//TODO: Allow for this?
			//swapchainCreateinfo.oldSwapchain = 0;

			VkSwapchainKHR swapchain;
			vkCreateSwapchainKHR(initStruct.device,&swapchainCreateinfo,NULL,&swapchain);

			vkGetSwapchainImagesKHR(initStruct.device, swapchain, swapchainImageCount, NULL);
			*swapchainImages = malloc(sizeof(VkImage) * (*swapchainImageCount));
			vkGetSwapchainImagesKHR(initStruct.device, swapchain, swapchainImageCount, *swapchainImages);

			if (swapchainImageViews)
			{
				*swapchainImageViews = malloc(sizeof(VkImageView) * (*swapchainImageCount));
				for (uint32_t i = 0; i < *swapchainImageCount; ++i)
				{
					VkImageViewCreateInfo swapchainImageViewCreateInfo = { 0 };
					swapchainImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					swapchainImageViewCreateInfo.image = (*swapchainImages)[i];
					swapchainImageViewCreateInfo.components = (VkComponentMapping){ VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A };
					swapchainImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					swapchainImageViewCreateInfo.format = chosenFormat.format;
					swapchainImageViewCreateInfo.subresourceRange.levelCount = 1;
					swapchainImageViewCreateInfo.subresourceRange.layerCount = 1;
					swapchainImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
					swapchainImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
					swapchainImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

					vkCreateImageView(initStruct.device,&swapchainImageViewCreateInfo,NULL,&((*swapchainImageViews)[i]));
				}
			}

			return swapchain;
		}
		else
		{
			free(surfaceFormats);
			free(presentModes);
			return VK_NULL_HANDLE;
		}

		free(surfaceFormats);
		free(presentModes);
	}
	else
		return VK_NULL_HANDLE;
}

#endif