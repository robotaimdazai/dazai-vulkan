#include "engine.h"

dazai_engine::engine::engine()
{
	m_glfw_window = new glfw_window();
	m_renderer = new renderer();
}

dazai_engine::engine::~engine()
{
	delete m_glfw_window;
	delete m_renderer;
}

auto dazai_engine::engine::update() -> void
{
	
	while (m_glfw_window->is_running())
	{
		glfwPollEvents();
	}
	
}
