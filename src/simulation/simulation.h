#pragma once
#include "../engine/defines.h"
#include "../engine/shared_structs.h"
#include <vector>
#include <GLFW/glfw3.h>

uint32_t constexpr MAX_ENTITIES = 1000;

struct entity
{
	transform transform;
};

struct simulation_state
{
	uint32_t entity_count;
	entity entities[MAX_ENTITIES];
};

class simulation
{
public:
	simulation(simulation_state * state, GLFWwindow* window);
	~simulation();
	auto create_entity(transform transform) -> entity*;
	auto update() -> void;
	auto handleMouseClick(double xpos, double ypos) -> void;
private:

	simulation_state* m_state;
	GLFWwindow* m_window;
};