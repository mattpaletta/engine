#pragma once

#include "shader.hpp"
#include "texture.hpp"

class PostProcessor {
public:
	// state
	Shader PostProcessingShader;
	Texture2D Texture;
	unsigned int Width, Height;

	// constructor
	PostProcessor(Shader shader, unsigned int width, unsigned int height);

	// prepares the postprocessor's framebuffer operations before rendering the game
	virtual void BeginRender();

	// should be called after rendering the game, so it stores all the rendered data into a texture object
	virtual void EndRender();

	// renders the PostProcessor texture quad (as a screen-encompassing large sprite)
	virtual void Render(float time);

protected:
	// render state
	unsigned int MSFBO, FBO; // MSFBO = Multisampled FBO. FBO is regular, used for blitting MS color-buffer to texture
	unsigned int RBO; // RBO is used for multisampled color buffer
	unsigned int VAO;

	// initialize quad for rendering postprocessing texture
	void initRenderData();
};