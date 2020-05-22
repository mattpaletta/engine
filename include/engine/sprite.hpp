#pragma once

#include <glm/glm.hpp>

#include <memory>

// Get Constants
#include <constants/colour.hpp>
#include <constants/size.hpp>
#include <constants/position.hpp>

#include "engine_fwd.hpp"
#include "renderer.hpp"

#include <constants/shader.hpp>
#include <constants/texture.hpp>

class SpriteRenderer : Renderer {
private:
	friend Engine;
	friend std::unique_ptr<SpriteRenderer>;
	friend std::shared_ptr<SpriteRenderer>;

	Shader shader;
	unsigned int quadVAO;

	// Initializes and configures the quad's buffers and vertex attributes
	void initRenderData();


public:
	// Create with the default shader
	SpriteRenderer();
	SpriteRenderer(const Shader& shader);
	
	static std::unique_ptr<SpriteRenderer> UniqueFromCustomShader(const Shader& shader) {
		return std::make_unique<SpriteRenderer>(shader);
	}

	static SpriteRenderer FromCustomShader(const Shader& shader) {
		return shader;
	}

	~SpriteRenderer();

	void DrawSprite(const Texture2D& texture, const glm::vec2& position, const Size& size = Size(10.0f, 10.0f), const float& rotate = 0.0f, const Colour& color = Colour::white);
};