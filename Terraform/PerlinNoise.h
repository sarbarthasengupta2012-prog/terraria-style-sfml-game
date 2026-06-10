#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <concepts> // For modern C++20 constraints

class PerlinNoise {
private:
    std::vector<float> m_gradients;

    [[nodiscard]] constexpr float fade(float t) const noexcept {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    [[nodiscard]] constexpr float lerp(float t, float a, float b) const noexcept {
        return std::lerp(a, b, t);
    }

public:
    explicit PerlinNoise(unsigned int seed) {
        std::mt19937 gen{ seed };
        std::uniform_real_distribution<float> dist{ -1.0f, 1.0f };

        m_gradients.resize(256);
        for (auto& gradient : m_gradients) {
            gradient = dist(gen);
        }
    }
    [[nodiscard]] float getNoise(float x) const noexcept {
        const float x_floor = std::floor(x);

        const int X0 = static_cast<int>(x_floor) & 255;
        const int X1 = (X0 + 1) & 255;

        const float dist0 = x - x_floor;
        const float dist1 = dist0 - 1.0f;

        const float influence0 = m_gradients[X0] * dist0;
        const float influence1 = m_gradients[X1] * dist1;

        const float u = fade(dist0);

        const float rawNoise = lerp(u, influence0, influence1);
        return (rawNoise + 1.0f) * 0.5f;
    }
};