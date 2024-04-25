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
		//sc image views
		std::vector<VkImageView> sc_image_views;
		//renderpass
		VkRenderPass render_pass;
		//framebuffers
		std::vector<VkFramebuffer> frame_buffers;
		//command pool
		VkCommandPool command_pool;
		//semaphores
		VkSemaphore acquire_semaphore{};
		VkSemaphore submit_semaphore{};
		//fences
		VkFence submit_queue_fence{};
		//queue family indices
		std::optional<uint32_t> graphic_family_queue_index;
		VkQueue graphics_queue;
	};

	class renderer
	{
	public:
		renderer(glfw_window* window);
		~renderer();
		auto init() -> void;
		auto render() -> bool;

	private:
		glfw_window* m_window;
		vk_context m_context;
	};
}