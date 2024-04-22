#pragma once
#include <vulkan/vulkan.h>
namespace dazai_engine
{
	class renderer
	{
	public:
		renderer();
		~renderer();
		auto init() -> void;

	private:
		VkInstance m_instance;
	};
}