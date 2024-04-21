#include "engine.h"

dazai_engine::engine::engine()
{
	build_glfw_window();
	create_vulkan_instance();
}

dazai_engine::engine::~engine()
{
	vkDestroyInstance(m_vk_instance, nullptr);
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

auto dazai_engine::engine::build_glfw_window() -> void
{
	//initialize glfw
	glfwInit();

	//no default rendering client, we'll hook vulkan up
	//to the window later
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//resizing breaks the swapchain, we'll disable it for now
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//GLFWwindow* glfwCreateWindow (int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share)
	if (m_window = glfwCreateWindow(m_width, m_height, "DazaiEngine", nullptr, nullptr))
	{
		
	}
	else
	{
		
	}

}

auto dazai_engine::engine::create_vulkan_instance()->void
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
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	create_info.enabledLayerCount = 0;
	VkResult result = vkCreateInstance(&create_info, nullptr, &m_vk_instance);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create VK instance");
	
}
