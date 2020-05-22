#pragma once

#include <map>
#include <glm/glm.hpp>

#include "engine_fwd.hpp"

#include <constants/texture.hpp>
#include <constants/shader.hpp>
#include <constants/position.hpp>
#include <constants/screen_size.hpp>
#include <constants/colour.hpp>

namespace {
	/// Holds all state information relevant to a character as loaded using FreeType
	struct Character {
		unsigned int TextureID; // ID handle of the glyph texture
		glm::ivec2   Size;      // size of glyph
		glm::ivec2   Bearing;   // offset from baseline to left/top of glyph
		signed int Advance;   // horizontal offset to advance to next glyph
	};
}

// A renderer class for rendering text displayed by a font loaded using the 
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class TextRenderer {
public:
	const static unsigned int defaultFontSize = 12;

	// Only Engine can call Init.
	friend Engine;

	// constructor
	TextRenderer();

	// pre-compiles a list of characters from the given font
	void Load(const std::string& font, const unsigned int& fontSize);

	// renders a string of text using the precompiled list of characters
	void RenderText(Engine* engine, const std::string& text, const Position2d& pos, const float& scale = 1.0f, const Colour& color = Colour::white);

	bool isFontLoaded(const std::string& font, const unsigned int& fontSize = defaultFontSize);
private:
	std::string currentFont = "";
	unsigned int currentFontSize = 0;

	// holds a list of pre-compiled Characters
	std::map<char, Character> Characters;

	// Doesn't use the engine yet, but left for future compatability
	void Init(Engine* engine, const ScreenSize& size, const std::string& font = "", const unsigned int& fontSize = defaultFontSize);

	// render state
	unsigned int VAO, VBO;

	// TODO: Support custom text renderers.
	Shader shader;
};