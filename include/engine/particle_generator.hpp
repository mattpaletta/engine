#pragma once
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "particle.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "game_object.hpp"

// ParticleGenerator acts as a container for rendering a large number of
// particles by repeatedly spawning and updating particles and killing
// them after a given amount of time.
class ParticleGenerator {
public:
    // constructor
    ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);

    // update all particles
    void Update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

    // render all particles
    void Draw();
private:
    // state
    std::vector<Particle> particles;
    unsigned int amount;
    // render state
    Shader shader;
    Texture2D texture;
    unsigned int VAO;

    // initializes buffer and vertex attributes
    void init();

    // returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
    unsigned int firstUnusedParticle();

    // respawns particle
    void respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};