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
		renderer* m_renderer;
		glfw_window* m_glfw_window;
	};
}
