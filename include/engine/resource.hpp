#pragma once
#include <string>
#include <map>

#if ENGINE_DEBUG
#include <set>
#endif

#include "engine_fwd.hpp"

#include <constants/cubemap.hpp>
#include <constants/shader.hpp>
#include <constants/texture.hpp>

#define ResourceManagerGetShader(name) \
    ResourceManager::GetShader(name, __FILE__, __LINE__)

class ResourceManager {
public:
    // resource storage
    std::map<std::string, Shader> Shaders;
    std::map<std::string, Texture2D> Textures;
    std::map<std::string, std::string> Sounds;

#if ENGINE_DEBUG
    std::set<std::string> UnusedShaders;
    std::set<std::string> UnusedTextures;
    std::set<std::string> UnusedSounds;
#endif

    // loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
    Shader& LoadShader(const std::string& vShaderFile, const std::string& fShaderFile, const std::string& name);

    // retrieves a stored sader
    Shader& GetShader(const std::string& name);
    Shader& GetShader(const std::string& name, const std::string& file, const std::size_t& line);

    void SetShaderAsSelfUsed(const std::string& name);

    // loads (and generates) a texture from file
    Texture2D& LoadTexture(const std::string& file, const std::string& name);
    Texture2D& LoadCubeMap(const CubeMap& faces, const bool flip_vertically, const std::string& name);

    // retrieves a stored texture
    Texture2D& GetTexture(const std::string& name);
    void SetTextureAsSelfUsed(const std::string& name);

    std::string RegisterSound(const std::string& file, const std::string& name);
    std::string& GetSound(const std::string& name);

    
private:
    friend Engine;

    // properly de-allocates all loaded resources
    void Clear();

    // private constructor, only the engine can create the resource manager
    ResourceManager() = default;

    // Calls clear before shutting down.
    ~ResourceManager();

    // loads and generates a shader from file
    Shader loadShaderFromFile(const std::string& vShaderFile, const std::string& fShaderFile);

    void loadImageFile(unsigned char* data, ScreenSize* size, int* nrChannels, const std::string& file, const bool flip_vertically = true);

    // loads a single texture from file
    Texture2D loadTextureFromFile(const std::string& file, const bool flip_vertically = true);

    // loads a cubemap from a file
    Texture2D loadCubeMapTextureFromFile(const CubeMap& faces, const bool flip_vertically = true);
};