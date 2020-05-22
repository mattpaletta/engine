#pragma once

#include <glm/glm.hpp>

#include "sprite.hpp"

#include <constants/colour.hpp>
#include <constants/texture.hpp>

class GameObject {
public:
#if !ENGINE_MIN_GAME_OBJECT
	// Optional Object State
	glm::vec2 position;
	glm::vec2 size;
	glm::vec2 velocity;

	Colour colour;
#endif

	GameObject() = default;

	// Moveable and copyable
	GameObject(const GameObject&) = default;
	GameObject& operator=(const GameObject&) = default;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	virtual void Draw(Renderer* renderer) const;
};