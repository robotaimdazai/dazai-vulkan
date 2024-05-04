#include "simulation.h"
#include "../engine/logger.h"
#include <random>


constexpr int SCREEN_WIDTH = 1280; // Example width
constexpr int SCREEN_HEIGHT = 720; // Example height
constexpr float GRAVITY = 0.5f;     // Example gravity value

simulation::simulation(simulation_state* state):
	m_state(state)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> width_dist(0, SCREEN_WIDTH);
    std::uniform_int_distribution<int> height_dist(0, SCREEN_HEIGHT);

    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        // Generate random coordinates within the screen bounds
        int x = width_dist(gen);
        int y = height_dist(gen);

        // Assuming you have a transform struct with x and y coordinates
        transform entityTransform;
        entityTransform.x = x;
        entityTransform.y = y;
        entityTransform.size_x = 10;
        entityTransform.size_y = 10;

        // Create an entity with the generated transform
        create_entity(entityTransform);
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
    for (int i = 0; i < m_state->entity_count; ++i)
    {
        // Apply gravity
        m_state->entities[i].transform.y += GRAVITY;

        // Check collision with floor
        if (m_state->entities[i].transform.y >= SCREEN_HEIGHT - m_state->entities[i].transform.size_y)
        {
            // If entity reaches the floor, reset its position
            m_state->entities[i].transform.y = SCREEN_HEIGHT - m_state->entities[i].transform.size_y;
        }

        // Check collision with other entities
        for (int j = 0; j < m_state->entity_count; ++j)
        {
            if (i != j) // Don't check collision with itself
            {
                if (check_collision(m_state->entities[i], m_state->entities[j]))
                {
                    // Handle collision between entities i and j
                    // Reverse their positions based on collision direction
                    if (m_state->entities[i].transform.y < m_state->entities[j].transform.y)
                    {
                        m_state->entities[i].transform.y -= GRAVITY;
                        m_state->entities[j].transform.y += GRAVITY;
                    }
                    else
                    {
                        m_state->entities[i].transform.y += GRAVITY;
                        m_state->entities[j].transform.y -= GRAVITY;
                    }
                }
            }
        }
    }
}

auto simulation::check_collision(const entity& a, const entity& b) -> bool
{
    // Check collision between two entities based on their bounding boxes
    return (a.transform.x < b.transform.x + b.transform.size_x &&
        a.transform.x + a.transform.size_x > b.transform.x &&
        a.transform.y < b.transform.y + b.transform.size_y &&
        a.transform.y + a.transform.size_y > b.transform.y);
}
