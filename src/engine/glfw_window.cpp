#include "glfw_window.h"
#include "logger.h"

dazai_engine::glfw_window::glfw_window()
{
	//initialize glfw
	glfwInit();
	//no default rendering client, we'll hook vulkan up
	//to the window later
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//resizing breaks the swapchain, we'll disable it for now
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	if (window = glfwCreateWindow(m_width, m_height, "DazaiEngine",
		nullptr, nullptr))
	{
		LOG_INFO("glfw window created");
	}
	else
	{
		LOG_ERROR("glfw window creation failed");
	}
} 

dazai_engine::glfw_window::~glfw_window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

auto dazai_engine::glfw_window::is_running() -> bool
{
	return !glfwWindowShouldClose(window);
}
