#pragma once

#include <glm/glm.hpp>

#include "colour.hpp"

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    Colour colour;
    float life;

    Particle() : position(0.0f), velocity(0.0f), colour(Colour::white), life(0.0f) { }
};