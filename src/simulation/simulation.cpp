#include "simulation.h"
#include "../engine/logger.h"
#include <random>
#include <cmath>
#include <chrono>

int SCREEN_WIDTH = 360; // Example width
int SCREEN_HEIGHT = 100; // Example height
constexpr float PARTICLE_RADIUS = 0.0f; // Example particle radius
constexpr float WATER_DENSITY = 0.00f; // Example water density
constexpr float GRAVITY = 9.8f; // Example gravity value
constexpr float DAMPING = 0.99f; // Example damping factor for air resistance
constexpr float COHESION_DISTANCE = 2.0f; // Example cohesion distance
constexpr float REPULSION_DISTANCE = 20.0f; // Example repulsion distance
float WAVE_AMPLITUDE = 0.3f; // Amplitude of the wave
float WAVE_FREQUENCY = 0.001f; // Frequency of the wave

namespace
{
    std::mt19937 m_random_engine(std::random_device{}());
    std::chrono::steady_clock::time_point m_nextWaveChangeTime = std::chrono::steady_clock::now() + std::chrono::seconds(5); // Initial wave parameter change after 5 seconds
    float m_targetAmplitude = WAVE_AMPLITUDE;
    float m_targetFrequency = WAVE_FREQUENCY;
    int m_transitionSteps = 0;
    int m_totalTransitionSteps = 0;
}

bool isSpacePressed(GLFWwindow* window)
{
    return glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
}

void simulation::handleMouseClick(double xpos, double ypos)
{
    transform newTransform;
    newTransform.x = static_cast<float>(xpos);
    newTransform.y = static_cast<float>(ypos);
    newTransform.size_x = 30; // Adjusted size
    newTransform.size_y = 30; // Adjusted size

    // Adjust y position to be above the highest particle beneath it
    for (int i = 0; i < m_state->entity_count; ++i)
    {
        if (m_state->entities[i].transform.x == newTransform.x)
        {
            if (m_state->entities[i].transform.y > newTransform.y)
            {
                newTransform.y = m_state->entities[i].transform.y + PARTICLE_RADIUS * 2.0f; // Ensure it's above the highest particle
            }
        }
    }

    create_entity(newTransform);
}

simulation::simulation(simulation_state* state, GLFWwindow* window) : m_state(state), m_window(window)
{
    std::uniform_real_distribution<float> width_dist(PARTICLE_RADIUS, SCREEN_WIDTH - PARTICLE_RADIUS);
    std::uniform_real_distribution<float> height_dist(PARTICLE_RADIUS, SCREEN_HEIGHT / 2); // Limit particles to top half of the screen

    for (int i = 0; i < MAX_ENTITIES / 2; ++i)
    {
        // Generate random coordinates within the top half of the screen
        float x = width_dist(m_random_engine);
        float y = height_dist(m_random_engine);

        // Assuming you have a transform struct with x and y coordinates
        transform entityTransform;
        entityTransform.x = x;
        entityTransform.y = y;
        entityTransform.size_x = 30; // Adjusted size
        entityTransform.size_y = 30; // Adjusted size

        // Create an entity with the generated transform
        create_entity(entityTransform);
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                simulation* sim = static_cast<simulation*>(glfwGetWindowUserPointer(window));
                sim->handleMouseClick(xpos, ypos);
            }
        });
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
    bool spacePressed = isSpacePressed(m_window);
    if (spacePressed)
    {
        SCREEN_WIDTH = 500;
        SCREEN_HEIGHT = 720;
    }
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

        // Reflective boundary conditions
        if (m_state->entities[i].transform.x < PARTICLE_RADIUS -10 || m_state->entities[i].transform.x > SCREEN_WIDTH - PARTICLE_RADIUS)
            m_state->entities[i].transform.x -= 2 * repulsion_force_x; // Reverse the x-component of the repulsion force

        if (m_state->entities[i].transform.y < PARTICLE_RADIUS || m_state->entities[i].transform.y > SCREEN_HEIGHT - PARTICLE_RADIUS)
            m_state->entities[i].transform.y -= 2 * repulsion_force_y; // Reverse the y-component of the repulsion force

        // Apply wave behavior to the y-coordinate
        float wave_amplitude = WAVE_AMPLITUDE * std::sin(WAVE_FREQUENCY * m_state->entities[i].transform.x);
        m_state->entities[i].transform.y += wave_amplitude;

        // Ensure particles stay within the screen boundaries
        if (m_state->entities[i].transform.y < PARTICLE_RADIUS)
            m_state->entities[i].transform.y = PARTICLE_RADIUS;
        else if (m_state->entities[i].transform.y > SCREEN_HEIGHT - PARTICLE_RADIUS)
            m_state->entities[i].transform.y = SCREEN_HEIGHT - PARTICLE_RADIUS;
    }

    // Check if it's time to change wave parameters
    auto now = std::chrono::steady_clock::now();
    if (now >= m_nextWaveChangeTime)
    {
        // Set new random targets for amplitude and frequency
        std::uniform_real_distribution<float> amplitude_dist(0.5f, 2.5f);
        std::uniform_real_distribution<float> frequency_dist(0.01f, 0.05f);
        m_targetAmplitude = amplitude_dist(m_random_engine);
        m_targetFrequency = frequency_dist(m_random_engine);

        // Set total transition steps and reset transition step count
        m_transitionSteps = 0;
        m_totalTransitionSteps = 1000; // Number of steps for the transition

        // Set next wave parameter change time
        m_nextWaveChangeTime = now + std::chrono::seconds(5 + rand() % 10); // Change parameters after 5 to 15 seconds
    }

    // Gradually adjust wave parameters towards the target
    if (m_transitionSteps < m_totalTransitionSteps)
    {
        constexpr float changeSpeed = 0.01f / 100.0f; // Speed of transition
        //WAVE_AMPLITUDE += changeSpeed * (m_targetAmplitude - WAVE_AMPLITUDE);
        //WAVE_FREQUENCY += changeSpeed * (m_targetFrequency - WAVE_FREQUENCY);
        ++m_transitionSteps;
    }
    else
    {
        // Ensure we reach the exact target values at the end of the transition
        //WAVE_AMPLITUDE = m_targetAmplitude;
        //WAVE_FREQUENCY = m_targetFrequency;
    }
}
