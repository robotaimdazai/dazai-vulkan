#pragma once
#include <vulkan/vulkan.h>

namespace dazai_engine
{
	struct image
	{
		VkImage vk_image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	struct buffer
	{
		VkBuffer vk_buffer;
		VkDeviceMemory memory;
		uint32_t size;
		void* data;
	};
}
