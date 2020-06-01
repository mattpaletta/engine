//
//  resource.cpp
//  engine
//
//  Created by Matthew Paletta on 2020-04-28.
//

#include "engine/resource.hpp"
#include <glad/glad.h>

#include <cstring>
#include <sstream>
#include <iostream>
#include <array>
#include <fstream>

#include <stb_image/stb_image.h>

#include <constants/filesystem.hpp>

ResourceManager::~ResourceManager() {
    this->Clear();
}

Shader& ResourceManager::LoadShader(const std::string& vShaderFile, const std::string& fShaderFile, const std::string& name) {
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile);
#if ENGINE_DEBUG
    UnusedShaders.insert(name);
#endif
    return Shaders.at(name);
}

Shader& ResourceManager::GetShader(const std::string& name) {
#if ENGINE_DEBUG
    if (!this->ShaderLoaded(name)) {
        std::cout << "Failed to get shader: " << name << std::endl;
    }
    UnusedShaders.erase(name);
#endif
    return Shaders.at(name);
}

Shader& ResourceManager::GetShader(const std::string& name, const std::string& file, const std::size_t& line) {
#if ENGINE_DEBUG
    if (!this->ShaderLoaded(name)) {
        std::cout << "Failed to get shader: " << name << " at: [" << file << ":" << line << "]" << std::endl;
    }

    UnusedShaders.erase(name);
#endif

    return Shaders.at(name);
}

bool ResourceManager::ShaderLoaded(const std::string& name) const {
	return Shaders.find(name) != Shaders.end();
}

void ResourceManager::SetShaderAsSelfUsed(const std::string& name) {
#if ENGINE_DEBUG
    UnusedShaders.erase(name);
#endif
}

Texture2D& ResourceManager::LoadTexture(const std::string& file,const std::string& name, const bool flip_vertically, const bool generate_mipmap) {
    if (!this->ShaderLoaded(name)) {
        // Only load if not found.
        Textures.emplace(name, loadTextureFromFile(file.c_str(), flip_vertically, generate_mipmap));

#if ENGINE_DEBUG
        UnusedTextures.insert(name);
#endif
    }
    return Textures.at(name);
}

Texture2D& ResourceManager::LoadCubeMap(const CubeMap& faces, const bool flip_vertically, const std::string& name) {
    Textures.emplace(name, loadCubeMapTextureFromFile(faces, flip_vertically));
#if ENGINE_DEBUG
    UnusedTextures.insert(name);
#endif
    return Textures.at(name);
}


Texture2D& ResourceManager::GetTexture(const std::string& name) {
    if (!this->TextureLoaded(name)) {
        std::cout << "Failed to get texture: " << name << std::endl;
    }
#if ENGINE_DEBUG
    UnusedTextures.erase(name);
#endif
    return Textures.at(name);
}

bool ResourceManager::TextureLoaded(const std::string& name) const {
    return Textures.find(name) != Textures.end();
}

void ResourceManager::SetTextureAsSelfUsed(const std::string& name) {
#if ENGINE_DEBUG
    UnusedTextures.erase(name);
#endif
}

std::string ResourceManager::RegisterSound(const std::string& file, const std::string& name) {
    Sounds.insert_or_assign(name, file);
#if ENGINE_DEBUG
    std::ifstream f(file);
    if (!f.good()) {
        std::cout << "Failed to open file: " << file << std::endl;
    }
    f.close();

    UnusedSounds.insert(name);
#endif
    return file;
}

std::string& ResourceManager::GetSound(const std::string& name) {
    if (Sounds.find(name) == Sounds.end()) {
        std::cout << "Failed to get sound: " << name << std::endl;
    }
#if ENGINE_DEBUG
    UnusedSounds.erase(name);
#endif
    return Sounds.at(name);
}

void ResourceManager::Clear() {
    // Warning about sounds
#if ENGINE_DEBUG
    for (const auto& iter : UnusedSounds) {
        std::cout << "Warning: sound loaded but never used: (" << iter << ")" << std::endl;
    }
#endif

    // (properly) delete all shaders
    for (const auto& iter : Shaders) {
#if ENGINE_DEBUG
        if (UnusedShaders.find(iter.first) != UnusedShaders.end()) {
            std::cout << "Warning: shader loaded but never used: " << iter.first << std::endl;
        }
#endif
        glDeleteProgram(iter.second.id());
    }

    // (properly) delete all textures
    for (const auto& iter : Textures) {
#if ENGINE_DEBUG
        if (UnusedTextures.find(iter.first) != UnusedTextures.end()) {
            std::cout << "Warning: texture loaded but never used: " << iter.first << std::endl;
        }
#endif
        glDeleteTextures(1, &iter.second.ID);
    }
}

Shader ResourceManager::loadShaderFromFile(const std::string& vShaderFile, const std::string& fShaderFile/*, const std::string& gShaderFile*/) {
    return Shader::from_file(vShaderFile, fShaderFile);
}

void ResourceManager::loadImageFile(unsigned char** data, ScreenSize* size, int* nrChannels, const std::string& file, const bool flip_vertically) {
#if ENGINE_DEBUG
    if (!constants::fs::exists(file)) {
        std::cerr << "ERROR: texture file not found - " << file << std::endl;
    }
#endif

    stbi_set_flip_vertically_on_load(flip_vertically);
    *data = stbi_load(file.c_str(), &size->WIDTH, &size->HEIGHT, nrChannels, STBI_default);

#if ENGINE_DEBUG
    if (!*data) {
        std::cout << "Error loading data" << std::endl;
    }
#endif
}


Texture2D ResourceManager::loadTextureFromFile(const std::string& file, const bool flip_vertically, const bool generate_mipmap) {
    // create texture object
    Texture2D texture;
    texture.Wrap_S = GL_REPEAT;
    texture.Wrap_T = GL_REPEAT;
	if (generate_mipmap) {
    	texture.Filter_Min = GL_LINEAR_MIPMAP_LINEAR;
	} else {
    	texture.Filter_Min = GL_LINEAR;
	}
    texture.Filter_Max = GL_LINEAR;

    ScreenSize size;
    unsigned char* data;
    int nrChannels;
    // This will not automatically unload it.
    this->loadImageFile(&data, &size, &nrChannels, file, flip_vertically);
    if (nrChannels == 1) {
        texture.Internal_Format = GL_RED;
        texture.Image_Format = GL_RED;
    } else if (nrChannels == 3) {
        texture.Internal_Format = GL_RGB;
        texture.Image_Format = GL_RGB;
    } else if (nrChannels == 4) {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    } else {
		std::cout << "ResourceManager::Texture::Error::Unknown number of channels" << std::endl;
    }

    // now generate texture
    texture.Generate(size, data, generate_mipmap);

    // and finally free image data
    stbi_image_free(data);
    return texture;
}


Texture2D ResourceManager::loadCubeMapTextureFromFile(const CubeMap& faces, const bool flip_vertically) {
    Texture2D texture;
    texture.Wrap_S = GL_CLAMP_TO_EDGE;
    texture.Wrap_T = GL_CLAMP_TO_EDGE;
    texture.Wrap_R = GL_CLAMP_TO_EDGE;
    texture.Filter_Min = GL_LINEAR;
    texture.Filter_Max = GL_LINEAR;

    // Get size from first image
    auto load_face = [this, &flip_vertically, &texture](const std::string& face, const std::size_t& index) {
        int nrChannels;
        ScreenSize size;
        unsigned char* data = nullptr;

        this->loadImageFile(&data, &size, &nrChannels, face, flip_vertically);
        if (index == 0) {
            if (nrChannels == 1) {
                texture.Internal_Format = GL_RED;
                texture.Image_Format = GL_RED;
            } else if (nrChannels == 3) {
                texture.Internal_Format = GL_RGB;
                texture.Image_Format = GL_RGB;
            } else if (nrChannels == 4) {
                texture.Internal_Format = GL_RGBA;
                texture.Image_Format = GL_RGBA;
            } else {
                texture.Internal_Format = GL_RGB;
                texture.Image_Format = GL_RGB;
            }
            // We initialize with the first one
            texture.GenerateCubeMapInit(size);
        }

        if (data) {
            texture.GenerateCubeMapFace(static_cast<int>(index), data);
        }
        stbi_image_free(data);
    };

    const std::array<std::string, 6> cubeFaces { {
        faces.right,  // +X
        faces.left,   // -X
        faces.top,    // +Y
        faces.bottom, // -Y
        faces.front,  // +Z
        faces.bottom  // -Z
    }};

    for (std::size_t i = 0; i < cubeFaces.size(); ++i) {
        load_face(cubeFaces.at(i), i);
    }
    texture.GenerateCubeMapCleanup();
    return texture;
}
