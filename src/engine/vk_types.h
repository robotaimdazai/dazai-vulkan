#pragma once
#include <vulkan/vulkan.h>

namespace dazai_engine
{
	struct image
	{
		VkImage vk_image;
		VkDeviceMemory memory;
	};
}
