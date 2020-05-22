#pragma once
#include <glm/glm.hpp>

class Colour {
private:
	float R;
	float G;
	float B;

	constexpr Colour(const float& _R, const float& _G, const float& _B) : R(_R), G(_G), B(_B) {}

public:
	static Colour white;
	static Colour red;
	static Colour blue;
	static Colour green;
	static Colour black;

	constexpr Colour() : R(0), G(0), B(0) {}
	constexpr Colour(const int hexValue) : R(((hexValue >> 16) & 0xFF) / 255.0f), G(((hexValue >> 8) & 0xFF) / 255.0f), B(((hexValue) & 0xFF) / 255.0f) {}

	constexpr static Colour from_hex(const int hexValue) {
		auto R = ((hexValue >> 16) & 0xFF) / 255.0f;  // Extract the RR byte
		auto G = ((hexValue >> 8) & 0xFF) / 255.0f;   // Extract the GG byte
		auto B = ((hexValue) & 0xFF) / 255.0f;        // Extract the BB byte
		return Colour(R, G, B);
	}

	glm::vec3 to_vec3() const;
};