#pragma once
#include "../engine/defines.h"
#include "../engine/shared_structs.h"
#include <vector>

uint32_t constexpr MAX_ENTITIES = 100;

struct entity
{
	transform transform;
	float velocity_x;
	float velocity_y;
};

struct simulation_state
{
	uint32_t entity_count;
	entity entities[MAX_ENTITIES];
};

class simulation
{
public:
	simulation(simulation_state * state);
	~simulation();
	auto create_entity(transform transform) -> entity*;
	auto update() -> void;
private:

	simulation_state* m_state;
};