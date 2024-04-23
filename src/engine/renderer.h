#pragma once
#include "glfw_window.h"
#include <vulkan/vulkan.h>
#include <optional>

namespace dazai_engine
{
	struct vk_context
	{
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physical_device;
		VkDevice device;
		//queue family indices
		std::optional<uint32_t> graphic_family_queue_index;
	};

	class renderer
	{
	public:
		renderer(GLFWwindow* window);
		~renderer();
		auto init() -> void;
		auto render() -> void;

	private:
		GLFWwindow* m_window;
		vk_context m_context;
	};
}