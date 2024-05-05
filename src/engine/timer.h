#pragma once

#include <chrono>

class timer
{
public:
    // Get delta time in seconds
    static float get_delta_time()
    {
        auto currentFrameTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentFrameTime - m_lastFrameTime).count();
        m_lastFrameTime = currentFrameTime;
        return deltaTime;
    }

private:
    static std::chrono::steady_clock::time_point m_lastFrameTime;
};

std::chrono::steady_clock::time_point timer::m_lastFrameTime = std::chrono::steady_clock::now();
