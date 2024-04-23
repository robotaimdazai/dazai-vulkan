#pragma once 
#include <GLFW/glfw3.h>
#include <stdexcept>


namespace dazai_engine
{
	class glfw_window
	{
	public:
		glfw_window();
		~glfw_window();
		auto is_running() -> bool;
		GLFWwindow* window{ nullptr };
	private:
		int m_width{ 640 };
		int m_height{ 480 };
		
	};
}
