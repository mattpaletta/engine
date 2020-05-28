#pragma once

#include <constants/position.hpp>

#include <glm/glm.hpp>

struct VertexDataWindow {
	Position2d position;
	glm::vec2 texCoord;

	VertexDataWindow(const Position2d& pos, const glm::vec2 tex) : position(pos), texCoord(tex) {}
};