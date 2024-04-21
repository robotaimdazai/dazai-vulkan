#pragma once 
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h >
#include <stdexcept>


namespace dazai_engine
{
	class engine
	{
	public:
		engine();
		~engine();
	private:
		auto build_glfw_window() -> void;
		auto create_vulkan_instance() -> void;
		int m_width{ 640 };
		int m_height{ 480 };
		GLFWwindow* m_window{ nullptr };
		VkInstance m_vk_instance;
	};
}
