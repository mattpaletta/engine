#pragma once

#include <glm/glm.hpp>

struct Size {
	float Width;
	float Height;

	Size(const float& width, const float& height) : Width(width), Height(height) {}
	~Size();

	glm::vec2 to_vec2() const {
		return { this->Width, this->Height };
	}
};