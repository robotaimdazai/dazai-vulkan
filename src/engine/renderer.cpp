#include "renderer.h"
#include "glfw_window.h"
#include "logger.h"

dazai_engine::renderer::renderer()
{
	init();
}

dazai_engine::renderer::~renderer()
{
	vkDestroyInstance(m_instance, nullptr);
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
	VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
	if (result == VK_SUCCESS)
		LOG_INFO("VK instance created");
	else
		LOG_ERROR("VK instance creation failed");
}
