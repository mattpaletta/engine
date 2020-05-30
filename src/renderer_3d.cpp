#include "engine/3d_renderer.hpp"

void Renderer3D::setProjectionMatrix(const glm::mat4 _projection) {
	this->projection = _projection;
}

void Renderer3D::setViewMatrix(const glm::mat4& _view) {
	this->view = _view;
}

void Renderer3D::setCameraPosition(const glm::vec3& _cameraPos) {
    this->cameraPos = _cameraPos;
}

const glm::mat4& Renderer3D::getProjection() const {
	return this->projection;
}

const glm::mat4& Renderer3D::getView() const {
	return this->view;
}

const glm::vec3& Renderer3D::getCameraPos() const {
    return this->cameraPos;
}
