#include "simulation.h"
#include "../engine/logger.h"
#include <random>

simulation::simulation(simulation_state* state):
	m_state(state)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> disX(-5.0f, 5.0f); // Horizontal range [-5.0, 5.0]
	std::uniform_real_distribution<float> disY(15.0f, 20.0f); // Vertical range [15.0, 20.0]

	// Spawn entities
	for (int i = 0; i < MAX_ENTITIES; ++i)
	{
		float x = disX(gen); // Random x position within the given horizontal range
		float y = disY(gen); // Random y position within the given vertical range
		transform t = { x, y, 0.0f, 0.0f }; // Assuming z and w are not used for position
		create_entity(t);
	}

}
simulation::~simulation()
{

}

auto simulation::create_entity(transform transform) -> entity*
{
	entity* e = nullptr;
	if (m_state->entity_count < MAX_ENTITIES)
	{
		e = &m_state->entities[m_state->entity_count++];
		e->transform = transform;
	}
	else
		LOG_ERROR("Entities limit reached");

	return e;

}

auto simulation::update() -> void
{
	// Update entity positions (simple example: move them randomly)
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

	for (int i = 0; i < m_state->entity_count; ++i)
	{
		float dx = dis(gen) * 0.1f; // Random delta x within the range [-0.1, 0.1]
		float dy = dis(gen) * 0.1f; // Random delta y within the range [-0.1, 0.1]

		// Update entity position
		m_state->entities[i].transform.x += dx;
		m_state->entities[i].transform.y += dy;

		// Limit entities within a range (assuming a square area centered at 0,0 with side length of 20)
		m_state->entities[i].transform.x = std::max(-10.0f, std::min(10.0f, m_state->entities[i].transform.x));
		m_state->entities[i].transform.y = std::max(-10.0f, std::min(10.0f, m_state->entities[i].transform.y));
	}
}
