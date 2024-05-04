#pragma once
#include "../engine/defines.h"
#include "../engine/shared_structs.h"
#include <vector>

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
	simulation(simulation_state * state);
	~simulation();
	auto create_entity(transform transform) -> entity*;
	auto update() -> void;
	auto check_collision(const entity& a, const entity& b) -> bool;
private:

	simulation_state* m_state;
};