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

	struct descriptor_info
	{
		union
		{
			VkDescriptorBufferInfo buffer_info;
			VkDescriptorImageInfo image_info;
		};

		descriptor_info(VkSampler sampler, VkImageView image_view)
		{
			image_info.sampler = sampler;
			image_info.imageView = image_view;
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		descriptor_info(VkBuffer buffer)
		{
			buffer_info.buffer = buffer;
			buffer_info.offset = 0;
			buffer_info.range = VK_WHOLE_SIZE;
		}

		descriptor_info(VkBuffer buffer, uint32_t offset, uint32_t range)
		{
			buffer_info.buffer = buffer;
			buffer_info.offset = offset;
			buffer_info.range = range;

		}
	};
}
