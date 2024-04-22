#pragma once
#include "glfw_window.h"
#include "renderer.h"
namespace dazai_engine
{
	class engine
	{
	public:
		engine();
		~engine();
		auto update() -> void;
	private:
		glfw_window* m_glfw_window;
		renderer* m_renderer;
	};
}
