#include "texture.hpp"

#include <glad/glad.h>

Texture2D::Texture2D() : ID(0), size(0, 0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Wrap_R(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR) {}

void Texture2D::Generate(const ScreenSize& screen_size, unsigned char* data, const bool generate_mipmap) {
	glGenTextures(1, &this->ID);
	this->size = screen_size;

#if ENGINE_DEBUG
	if (this->size.WIDTH == 0 || this->size.HEIGHT == 0) {
		std::cerr << "Warning::Width or Height == 0" << std::endl;
	}
#endif
	// create Texture
	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, this->size.WIDTH, this->size.HEIGHT, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
	if (generate_mipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	// set Texture wrap and filter modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);

	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::GenerateMipmap(const ScreenSize& screen_size, unsigned char* data) {
	this->Generate(size, data, true);
}

void Texture2D::GenerateCubeMapInit(const ScreenSize& screen_size) {
	glGenTextures(1, &this->ID);
	this->size = screen_size;
}


void Texture2D::GenerateCubeMapFace(const int i, unsigned char* data) {
	// loads a cubemap texture from 6 individual texture faces
	// order:
	// +X (right)
	// -X (left)
	// +Y (top)
	// -Y (bottom)
	// +Z (front)
	// -Z (back)
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, this->Internal_Format, this->size.WIDTH, this->size.HEIGHT, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
}

void Texture2D::GenerateCubeMapCleanup() {
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, this->Wrap_S);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, this->Wrap_T);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, this->Wrap_R);
}

void Texture2D::Bind() const {
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

void Texture2D::BindCubeMap() const {
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->ID);
}
