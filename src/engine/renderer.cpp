#include "renderer.h"
#include "glfw_window.h"
#include "logger.h"
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_win32.h>
#include <vector>

dazai_engine::renderer::renderer(GLFWwindow* window):
	m_window(window)
{
	init();
}

dazai_engine::renderer::~renderer()
{
	vkDestroySurfaceKHR(m_context.instance, m_context.surface, nullptr);
	vkDestroyInstance(m_context.instance, nullptr);
	vkDestroyDevice(m_context.device, nullptr);
}

auto dazai_engine::renderer::init() -> void
{
	//app info
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Dazai Vulkan";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Dazai Engine";
	app_info.apiVersion = VK_API_VERSION_1_0;
	//global extensions and validation layer
	const char* layers[]
	{
		"VK_LAYER_KHRONOS_validation"
	};
	VkInstanceCreateInfo instance_info{};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	//glfw extensions
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	std::vector<const char*> extensions(glfw_extensions,
		glfw_extensions + glfw_extension_count);
	//add other extensions
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	instance_info.ppEnabledExtensionNames = extensions.data();
	instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_info.ppEnabledLayerNames = layers;
	instance_info.enabledLayerCount = ARRAYSIZE(layers);
	//CREATE INSTANCE
	VKCHECK(vkCreateInstance(&instance_info, nullptr, &m_context.instance));
	//ENABLE DEBUG MESSENGER
	//STEPS:
	//GET FUNCTION POINTER FROM DLL
	//CAST THE FUNCTION APPROPRIATELY
	//IF POINTER IS VALID CREATE MESSENGER
	auto debug_messenger_function_ptr =
		(PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(m_context.instance, "vkCreateDebugUtilsMessengerEXT");
	if (debug_messenger_function_ptr)
	{
		VkDebugUtilsMessengerCreateInfoEXT debug_info{};
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		//turn on bits for message type
		debug_info.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		//callback functiion
		debug_info.pfnUserCallback = VK_DEBUG_CALLBACK;
		debug_messenger_function_ptr(m_context.instance, &debug_info,
			0, &m_context.debug_messenger);
	}
	else
	{
		LOG_ERROR("DEBUG MESSENGER FUNCTION PTR NOT FOUND");
	}
	//create vulkan surface
	VkWin32SurfaceCreateInfoKHR surface_info{};
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hwnd = glfwGetWin32Window(m_window);
	surface_info.hinstance = GetModuleHandle(nullptr);
	VKCHECK(vkCreateWin32SurfaceKHR(m_context.instance, &surface_info, 0, &m_context.surface));
	//select physical device
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(m_context.instance, &device_count, nullptr);
	if (device_count == 0)
		LOG_ERROR("No GPU with Vulkan support found");
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(m_context.instance, &device_count, devices.data());
	if (device_count > 1)
		LOG_WARNING("You have more than one supported physical devices, set accordingly");
	//I only have 1 device so setting directly
	m_context.physical_device = devices[0];
	//check queue families in selected gpu
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_context.physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_context.physical_device, &queue_family_count, queue_families.data());
	int i = 0;
	for (const auto& queue_family: queue_families)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) // check if queue family has graphic queue
			m_context.graphic_family_queue_index = i;

		if (m_context.graphic_family_queue_index.has_value()) // if value was assigned break
			break;

		i++;
	}
	if (!m_context.graphic_family_queue_index.has_value())
	{
		LOG_ERROR("Graphic queue family failed");
		return;
	}
	//CREATE LOGICAL DEVICE
	//first we need to configure queue family we going to use in logical device
	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = m_context.graphic_family_queue_index.value();
	queue_create_info.queueCount = 1;
	float queue_priority = 1;
	queue_create_info.pQueuePriorities = &queue_priority;
	//configure physical device feature we will be using;
	VkPhysicalDeviceFeatures device_features{}; // nothing for now
	//create extensions for logical device
	const char* sc_extensions[] = 
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	//now create logical device
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.ppEnabledExtensionNames = sc_extensions;
	device_create_info.enabledExtensionCount = ARRAYSIZE(sc_extensions);
	VKCHECK(vkCreateDevice(m_context.physical_device,
		&device_create_info,0,&m_context.device));
	//Retrieving queue handles
	vkGetDeviceQueue(m_context.device,m_context.graphic_family_queue_index.value(),
		0,&m_context.graphics_queue);
	//create swapchain
	VkSwapchainCreateInfoKHR sc_info{};
	sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_info.surface = m_context.surface;
	VKCHECK(vkCreateSwapchainKHR(m_context.device, &sc_info, 0,
		&m_context.swap_chain));

}

auto dazai_engine::renderer::render() -> void
{

}



