#pragma once
#include <glm/glm.hpp>

class Colour {
private:
	constexpr Colour(const float& _R, const float& _G, const float& _B) : R(_R), G(_G), B(_B) {}
public:
	float R;
	float G;
	float B;

	static Colour white;
	static Colour red;
	static Colour blue;
	static Colour green;
	static Colour black;

	constexpr Colour() : R(0), G(0), B(0) {}
	constexpr Colour(const int hexValue) : 
		R(static_cast<float>((hexValue >> 16) & 0xFF) / 255.0f), 
		G(static_cast<float>((hexValue >> 8) & 0xFF) / 255.0f), 
		B(static_cast<float>((hexValue) & 0xFF) / 255.0f) {}

	bool operator==(const Colour& other) const noexcept;
	bool operator!=(const Colour& other) const noexcept;

	constexpr static Colour from_hex(const int hexValue) {
		const float R = static_cast<float>((hexValue >> 16) & 0xFF) / 255.0f;  // Extract the RR byte
		const float G = static_cast<float>((hexValue >> 8) & 0xFF) / 255.0f;   // Extract the GG byte
		const float B = static_cast<float>((hexValue) & 0xFF) / 255.0f;        // Extract the BB byte
		return Colour(R, G, B);
	}

	glm::vec3 to_vec3() const;
};