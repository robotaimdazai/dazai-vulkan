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
	vkDestroyInstance(m_context.instance, nullptr);
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
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	//glfw extensions
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	create_info.enabledExtensionCount = glfw_extension_count;
	create_info.ppEnabledExtensionNames = glfw_extensions;
	//validation layers
	create_info.enabledLayerCount = 0;
	VkResult result = vkCreateInstance(&create_info, nullptr, &m_context.instance);
	if (result == VK_SUCCESS)
		LOG_INFO("VK instance created");
	else
		LOG_ERROR("VK instance failed");
	//create vulkan surface
	VkWin32SurfaceCreateInfoKHR surface_info{};
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hwnd = glfwGetWin32Window(m_window);
	surface_info.hinstance = GetModuleHandle(nullptr);
	result = vkCreateWin32SurfaceKHR(m_context.instance,&surface_info,0,&m_context.surface);
	if (result == VK_SUCCESS)
		LOG_INFO("VK Surface created");
	else
		LOG_ERROR("VK Surface failed");
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
	//now createlogical device
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.enabledExtensionCount = 0;
	result = vkCreateDevice(m_context.physical_device,&device_create_info,0,&m_context.device);
	if (result == VK_SUCCESS)
		LOG_INFO("VK logical device created");
	else
		LOG_ERROR("VK logical device failed");

}

auto dazai_engine::renderer::render() -> void
{

}

