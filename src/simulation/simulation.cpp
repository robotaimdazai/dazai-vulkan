#include "simulation.h"
#include "../engine/logger.h"
#include <random>
#include <cmath>

constexpr int SCREEN_WIDTH = 1280; // Example width
constexpr int SCREEN_HEIGHT = 720; // Example height
constexpr float PARTICLE_RADIUS = 5.0f; // Example particle radius
constexpr float WATER_DENSITY = 0.05f; // Example water density
constexpr float GRAVITY = 9.8f; // Example gravity value
constexpr float DAMPING = 0.99f; // Example damping factor for air resistance
constexpr float COHESION_DISTANCE = 50.0f; // Example cohesion distance
constexpr float REPULSION_DISTANCE = 20.0f; // Example repulsion distance

simulation::simulation(simulation_state* state) : m_state(state)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> width_dist(PARTICLE_RADIUS, SCREEN_WIDTH - PARTICLE_RADIUS);
    std::uniform_real_distribution<float> height_dist(PARTICLE_RADIUS, SCREEN_HEIGHT / 2); // Limit particles to top half of the screen

    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        // Generate random coordinates within the top half of the screen
        float x = width_dist(gen);
        float y = height_dist(gen);

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

simulation::~simulation() {}

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
    // Apply gravity and forces, and update entity positions
    for (int i = 0; i < m_state->entity_count; ++i)
    {
        // Apply gravity
        m_state->entities[i].transform.y += GRAVITY;

        // Apply damping (air resistance)
        m_state->entities[i].transform.y *= DAMPING;

        // Calculate repulsion forces from neighboring particles
        float repulsion_force_x = 0.0f;
        float repulsion_force_y = 0.0f;
        for (int j = 0; j < m_state->entity_count; ++j)
        {
            if (i != j)
            {
                // Calculate distance between entities
                float dx = m_state->entities[j].transform.x - m_state->entities[i].transform.x;
                float dy = m_state->entities[j].transform.y - m_state->entities[i].transform.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Apply repulsion force if particles are within repulsion distance
                if (distance < REPULSION_DISTANCE)
                {
                    float factor = 1.0f - (distance / REPULSION_DISTANCE);
                    repulsion_force_x -= factor * dx;
                    repulsion_force_y -= factor * dy;
                }
            }
        }

        // Apply repulsion force to adjust particle position
        m_state->entities[i].transform.x += repulsion_force_x;
        m_state->entities[i].transform.y += repulsion_force_y;

        // Prevent particles from going beyond the screen boundaries
        if (m_state->entities[i].transform.x < PARTICLE_RADIUS)
            m_state->entities[i].transform.x = PARTICLE_RADIUS;
        else if (m_state->entities[i].transform.x > SCREEN_WIDTH - PARTICLE_RADIUS)
            m_state->entities[i].transform.x = SCREEN_WIDTH - PARTICLE_RADIUS;

        if (m_state->entities[i].transform.y < PARTICLE_RADIUS)
            m_state->entities[i].transform.y = PARTICLE_RADIUS;
        else if (m_state->entities[i].transform.y > SCREEN_HEIGHT - PARTICLE_RADIUS)
            m_state->entities[i].transform.y = SCREEN_HEIGHT - PARTICLE_RADIUS;
    }
 }
