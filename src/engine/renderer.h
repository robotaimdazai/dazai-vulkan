#pragma once
#include "glfw_window.h"
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
namespace dazai_engine
{
	struct vk_context
	{
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
		//surface
		VkSurfaceKHR surface;
		VkSurfaceFormatKHR surface_format;
		//devices
		VkPhysicalDevice physical_device;
		VkDevice device;
		// swap chain
		VkSwapchainKHR swap_chain;
		uint32_t sc_image_count;
		std::vector<VkImage> sc_images;
		//command pool
		VkCommandPool command_pool;
		//semaphores
		VkSemaphore acquire_semaphore{};
		VkSemaphore submit_semaphore{};
		//queue family indices
		std::optional<uint32_t> graphic_family_queue_index;
		VkQueue graphics_queue;
	};

	class renderer
	{
	public:
		renderer(GLFWwindow* window);
		~renderer();
		auto init() -> void;
		auto render() -> bool;

	private:
		GLFWwindow* m_window;
		vk_context m_context;
	};
}