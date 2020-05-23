//
//  resource.cpp
//  engine
//
//  Created by Matthew Paletta on 2020-04-28.
//

#include "engine/resource.hpp"
#include <glad/glad.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <stb_image/stb_image.h>

ResourceManager::~ResourceManager() {
    this->Clear();
}

Shader& ResourceManager::LoadShader(const std::string& vShaderFile, const std::string& fShaderFile, const std::string& gShaderFile, const std::string& name) {
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
#if ENGINE_DEBUG
    UnusedShaders.insert(name);
#endif
    return Shaders.at(name);
}

Shader& ResourceManager::GetShader(const std::string& name) {
#if ENGINE_DEBUG
    if (Shaders.find(name) == Shaders.end()) {
        std::cout << "Failed to get shader: " << name << std::endl;
    }
    UnusedShaders.erase(name);
#endif
    return Shaders.at(name);
}

Shader& ResourceManager::GetShader(const std::string& name, const std::string& file, const std::size_t& line) {
#if ENGINE_DEBUG
    if (Shaders.find(name) == Shaders.end()) {
        std::cout << "Failed to get shader: " << name << " at: [" << file << ":" << line << "]" << std::endl;
    }

    UnusedShaders.erase(name);
#endif

    return Shaders.at(name);
}

void ResourceManager::SetShaderAsSelfUsed(const std::string& name) {
#if ENGINE_DEBUG
    UnusedShaders.erase(name);
#endif
}

Texture2D& ResourceManager::LoadTexture(const std::string& file, const bool alpha, const std::string& name) {
    Textures.emplace(name, loadTextureFromFile(file.c_str(), alpha));
#if ENGINE_DEBUG
    UnusedTextures.insert(name);
#endif
    return Textures.at(name);
}

Texture2D& ResourceManager::GetTexture(const std::string& name) {
    if (Textures.find(name) == Textures.end()) {
        std::cout << "Failed to get texture: " << name << std::endl;
    }
#if ENGINE_DEBUG
    UnusedTextures.erase(name);
#endif
    return Textures.at(name);
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

Shader ResourceManager::loadShaderFromFile(const std::string& vShaderFile, const std::string& fShaderFile, const std::string& gShaderFile) {
    return Shader::from_file(vShaderFile, fShaderFile, gShaderFile);
}

Texture2D ResourceManager::loadTextureFromFile(const std::string& file, const bool alpha, const bool flip_vertically) {
    // create texture object
    Texture2D texture;
    if (alpha) {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    // load image
    ScreenSize size;
    int nrChannels;

#if ENGINE_DEBUG
    if (!std::filesystem::exists(file)) {
        std::cerr << "ERROR: texture file not found - " << file << std::endl;
    }
#endif

    stbi_set_flip_vertically_on_load(flip_vertically);
    unsigned char* data = stbi_load(file.c_str(), &size.WIDTH, &size.HEIGHT, &nrChannels, STBI_default);

#if ENGINE_DEBUG
    if (strlen((char *) (data)) == 0) {
        std::cout << "Texture read no data" << std::endl;
    }
#endif

    // now generate texture
    texture.Generate(size, data);

    // and finally free image data
    stbi_image_free(data);
    return texture;
}