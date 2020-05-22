#include "engine/text_renderer.hpp"

#include <glad/glad.h>

#include <iostream>
#include <array>

#include <glm/gtc/matrix_transform.hpp>
#if ENGINE_ENABLE_TEXT
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "engine/resource.hpp"
#include "engine/engine.hpp"

const std::string projection_var = "projection";
const std::string output_colour_var = "textColor";
const std::string char_glyph_var = "text";

Shader getDefaultTextShader() {
	const std::string tex_coords_var = "TexCoords";
	const std::string text_vert =
		"#version 330 core\n"
		"layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
		"out vec2 "+tex_coords_var+";\n"
		"\n"
		"uniform mat4 "+projection_var+";\n"
		"\n"
		"void main() {\n"
		"	gl_Position = "+projection_var+" * vec4(vertex.xy, 0.0, 1.0);\n"
		"	"+tex_coords_var+" = vertex.zw;\n"
		"}";

	const std::string colour_var = "colour";
	const std::string text_frag =
		"#version 330 core\n"
		"in vec2 "+tex_coords_var+";\n"
		"out vec4 "+colour_var+";\n"
		"\n"
		"uniform sampler2D "+char_glyph_var+";\n"
		"uniform vec3 "+output_colour_var+";\n"
		"\n"
		"void main() {\n"
		"	vec4 sampled = vec4(1.0, 1.0, 1.0, texture("+char_glyph_var+", "+tex_coords_var+").r);\n"
		"	"+colour_var+" = vec4("+output_colour_var+", 1.0) * sampled;\n"
		"}";

	return Shader(text_vert, text_frag);
}

TextRenderer::TextRenderer() : shader(getDefaultTextShader()) {}

void TextRenderer::Init(Engine* engine, const ScreenSize& size, const std::string& font, const unsigned int& fontSize) {
#if ENGINE_ENABLE_TEXT
	// load and configure shader
	this->shader.use().
		setMat4(projection_var, glm::ortho(0.0f, static_cast<float>(size.WIDTH), static_cast<float>(size.HEIGHT), 0.0f)).
		setInt(char_glyph_var, 0);

	// configure VAO/VBO for texture quads
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (font != "" && fontSize > 0) {
		this->Load(font, fontSize);
	}
#endif
}

bool TextRenderer::isFontLoaded(const std::string& font, const unsigned int& fontSize) {
#if ENGINE_ENABLE_TEXT
	return this->currentFont == font && this->currentFontSize == fontSize;
#else
	return false;
#endif
}

void TextRenderer::Load(const std::string& font, const unsigned int& fontSize) {
#if ENGINE_ENABLE_TEXT
	if (this->isFontLoaded(font, fontSize)) {
		// Font is already loaded, do nothing
		return;
	}

	// first clear the previously loaded Characters
	this->Characters.clear();

	// then initialize and load the FreeType library
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) { // all functions return a value different than 0 whenever an error occurred
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, font.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}

	// set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	// disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// then for the first 128 ASCII characters, pre-load/compile their characters and store them
	for (GLubyte c = 0; c < 128; ++c) {
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// now store character for later use
		Character character {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	this->currentFont = font;
	this->currentFontSize = fontSize;
#endif
}

void TextRenderer::RenderText(Engine* engine, const std::string& text, const Position2d& pos, const float& scale, const Colour& color) {
#if ENGINE_ENABLE_TEXT
	// activate corresponding render state
	this->shader.use().\
		setVec3(output_colour_var, color.to_vec3());

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(this->VAO);

	Position2d currPos = pos;

	// iterate through all characters
	for (const auto& c : text) {
		const Character ch = Characters[c];

		const float xpos = currPos.x + ch.Bearing.x * scale;
		// Use `H` because it touches the top and bottom of the line
		const float ypos = currPos.y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;

		const float w = ch.Size.x * scale;
		const float h = ch.Size.y * scale;

		// update VBO for each character
		const float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 0.0f },

			{ xpos,     ypos + h,   0.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// now advance cursors for next glyph
		currPos.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
#endif
}