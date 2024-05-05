#include "engine.h"
#include "logger.h"
#include "../simulation/simulation.h"

dazai_engine::engine::engine()
{
	m_glfw_window = new glfw_window();
	m_renderer = new renderer(m_glfw_window);
}

dazai_engine::engine::~engine()
{
	delete m_glfw_window;
	delete m_renderer;
}

auto dazai_engine::engine::update() -> void
{
	
	simulation_state s_state{};
	simulation simulation(&s_state,m_glfw_window->window);

	while (m_glfw_window->is_running())
	{
		//update simulation
		simulation.update();
		//render loop
		bool success = m_renderer->render(&s_state);
		if (!success)
		{
			LOG_ERROR("Render loop failed");
		}
		//event polling
		glfwPollEvents();
	}
	
}
