#pragma once
#include <glad/glad.h>
#include "glm/glm.hpp"

#include "engine/renderer.hpp"

class Renderer3D : public Renderer {
private:
	glm::mat4 view;
	glm::mat4 projection;
public:
	void setProjectionMatrix(const glm::mat4 _projection);
	void setViewMatrix(const glm::mat4& _view);

	const glm::mat4& getProjection() const;
	const glm::mat4& getView() const;
};