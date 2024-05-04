#include "simulation.h"
#include "../engine/logger.h"
#include <random>

simulation::simulation(simulation_state* state):
	m_state(state)
{
	

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
	
}
