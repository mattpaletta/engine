#include "colour.hpp"

Colour Colour::white = Colour(0xFFFFFF);
Colour Colour::red = Colour(0xFF0000);
Colour Colour::blue = Colour(0x00FF00);
Colour Colour::green = Colour(0x0000FF);
Colour Colour::black = Colour();

glm::vec3 Colour::to_vec3() const {
	return { this->R, this->G, this->B };
}