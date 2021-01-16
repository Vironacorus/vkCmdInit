#define VKCMDINIT_IMPL
#define VKCMDINIT_GLFW
#define VK_USE_PLATFORM_WIN32_KHR
#include "init.h"
#include <stdio.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallbackProc(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		printf("Debug Callback: %s \n", pCallbackData->pMessage);

	return VK_FALSE;
}

void SurfaceOptions(const VkSurfaceCapabilitiesKHR* surfaceCapabilities, VkExtent2D* extent, uint32_t* imageCount)
{
	*extent = (VkExtent2D){ 200, 200 };
	*imageCount = surfaceCapabilities->minImageCount;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(200,200,"vkCmdInit", NULL, NULL);

	InitializationStruct initstruct = createApplication("None","X",VK_MAKE_VERSION(1,0,0),VK_MAKE_VERSION(1,0,0), VK_API_VERSION_1_0);
	initstruct = addExtension(addExtension(addExtension(initstruct, VK_EXT_DEBUG_UTILS_EXTENSION_NAME),VK_KHR_SURFACE_EXTENSION_NAME),VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	initstruct = withSurfaceGLFW(withDebugOutput1(startInstance(initstruct), DebugCallbackProc, NULL),window);
	
	const char* expectedDeviceExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	initstruct = selectPhysicalDevices(initstruct,NULL, NULL);
	initstruct = createDevice(initstruct, NULL, expectedDeviceExtensions, sizeof(expectedDeviceExtensions)/sizeof(expectedDeviceExtensions[0]));


	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	{
		DefaultQueueRetrieveStruct queues = { 0 };
		retrieveQueues(initstruct, &queues, NULL);
		graphicsQueue = queues.graphicsQueue;
		presentationQueue = queues.presentationQueue;
	}

	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	uint32_t swapchainImageCount;
	VkSwapchainKHR swapchain = createSwapchainKHR(initstruct,NULL,NULL, SurfaceOptions,&swapchainImageCount,&swapchainImages,&swapchainImageViews);



	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	for (uint32_t i = 0; i < swapchainImageCount; ++i)
		vkDestroyImageView(initstruct.device,swapchainImageViews[i],NULL);
	vkDestroySwapchainKHR(initstruct.device,swapchain,NULL);
	terminateInstance(initstruct);
	glfwTerminate();
}