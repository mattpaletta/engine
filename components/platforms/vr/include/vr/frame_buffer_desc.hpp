#pragma once
#include <glad/glad.h>

#include <constants/texture.hpp>

struct FramebufferDesc {
	GLuint depthBufferId;

	Texture2D renderTexture;
	GLuint renderTextureId;
	GLuint renderFramebufferId;

	Texture2D resolveTexture;
	GLuint resolveTextureId;
	GLuint resolveFramebufferId;
};