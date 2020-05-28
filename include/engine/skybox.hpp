#pragma once
#include "engine_fwd.hpp"
#include "game_object.hpp"
#include "3d_renderer.hpp"

#include <constants/shader.hpp>
#include <constants/texture.hpp>
#include <constants/cubemap.hpp>

#include <string>

class Skybox : GameObject {
private:
	unsigned int VAO;
	unsigned int VBO;

	mutable Shader shader;
	mutable Texture2D texture;

public:
	Skybox();
	~Skybox();

	void Init(Engine* engine, const CubeMap& cubemap);

	void Draw(Renderer3D* renderer) const noexcept;
};