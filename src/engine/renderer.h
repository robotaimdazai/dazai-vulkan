#pragma once
#include "glfw_window.h"
#include <optional>
#include <vector>
#include "vk_types.h"
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
		//pipeline
		VkPipeline pipeline;
		//pipeline layout
		VkPipelineLayout pipeline_layout;
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
		//staging buffer
		buffer staging_buffer;
		//descriptor pool
		VkSampler sampler;
		VkDescriptorPool descriptor_pool;
		//temporary
		//TODO: needs to be abstracted
		image image;
		VkDescriptorSetLayout set_layout;
		VkDescriptorSet descriptor_set;
	};

	class renderer
	{
	public:
		renderer(glfw_window* window);
		~renderer();
		auto init() -> bool;
		auto render() -> bool;
	private:
		auto alloc_image
		(VkDevice device,
			VkPhysicalDevice physical_device,
			uint32_t width,
			uint32_t height,
			VkFormat format) -> image;
		auto alloc_buffer(VkDevice device,
			VkPhysicalDevice physical_device,
			uint32_t size,
			VkBufferUsageFlags buffer_usage,
			VkMemoryPropertyFlags mem_props) -> buffer;
		auto get_memory_type_index(VkPhysicalDevice device,
			VkMemoryRequirements mem_reqs,
			VkMemoryPropertyFlags mem_props) -> uint32_t;
		auto cmd_begin_info() -> VkCommandBufferBeginInfo;
		auto cmd_alloc_info(VkCommandPool pool) -> VkCommandBufferAllocateInfo;
		auto fence_info(VkFenceCreateFlags flags = 0) -> VkFenceCreateInfo;
		auto submit_info(VkCommandBuffer* cmd, uint32_t cmd_count = 1) -> VkSubmitInfo;
		auto copy_to_buffer(buffer* buffer, void* data, uint32_t size) -> void;
		glfw_window* m_window;
		vk_context m_context;
	};
}