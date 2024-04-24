#include "engine.h"
#include "logger.h"

dazai_engine::engine::engine()
{
	m_glfw_window = new glfw_window();
	m_renderer = new renderer(m_glfw_window->window);
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
		bool success = m_renderer->render();
		if (!success)
		{
			LOG_ERROR("Render loop failed");
		}
		glfwPollEvents();
	}
	
}
