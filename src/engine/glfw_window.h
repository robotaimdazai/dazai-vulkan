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
		const unsigned  int width{ 640 };
		const unsigned  int height{ 720 };
		
		
	};
}
