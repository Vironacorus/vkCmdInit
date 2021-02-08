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
	extent->width = 200;
	extent->height = 200;
	*imageCount = surfaceCapabilities->minImageCount;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(200,200,"vkCmdInit", nullptr, nullptr);
	
	vki::InitializationStruct initstruct = vki::createApplication("None","X",VK_MAKE_VERSION(1,0,0),VK_MAKE_VERSION(1,0,0), VK_API_VERSION_1_0);
	vki::addExtension(vki::addExtension(vki::addExtension(initstruct, VK_EXT_DEBUG_UTILS_EXTENSION_NAME),VK_KHR_SURFACE_EXTENSION_NAME),VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	vki::withSurfaceGLFW(vki::withDebugOutput(vki::startInstance(initstruct), DebugCallbackProc),window);
	
	const char* expectedDeviceExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vki::selectPhysicalDevices(initstruct);
	vki::createDevice(initstruct, nullptr, expectedDeviceExtensions, sizeof(expectedDeviceExtensions)/sizeof(expectedDeviceExtensions[0]));


	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	uint32_t* families;

	{
		vki::DefaultQueueRetrieveStruct queues{};
		vki::retrieveQueues(initstruct,queues,&families,nullptr);
		graphicsQueue = queues.graphicsQueue;
		presentationQueue = queues.presentationQueue;
	}

	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	uint32_t swapchainImageCount;
	VkSwapchainKHR swapchain = vki::createSwapchainKHR(initstruct, nullptr, nullptr, SurfaceOptions,swapchainImageCount,swapchainImages,swapchainImageViews);


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	for (uint32_t i = 0; i < swapchainImageCount; ++i)
		vkDestroyImageView(initstruct.device,swapchainImageViews[i], nullptr);
	vkDestroySwapchainKHR(initstruct.device,swapchain, nullptr);
	terminateInstance(&initstruct);
	glfwTerminate();
}