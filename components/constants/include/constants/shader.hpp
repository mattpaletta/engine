#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader {
private:
    // the program ID
    unsigned int ID;
public:

    // constructor reads and builds the shader
    Shader() : ID(0) {}
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = "");

    static Shader from_file(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");

    // use/activate the shader
    Shader& use();

    bool valid() const;

    const unsigned int id() const;

    // utility uniform functions
    const Shader& setBool(const std::string& name, bool value) const;
    const Shader& setInt(const std::string& name, int value) const;
    const Shader& setFloat(const std::string& name, float value) const;
    const Shader& setVec2(const std::string& name, const glm::vec2& value) const;
    const Shader& setVec2(const std::string& name, float x, float y) const;
    const Shader& setVec3(const std::string& name, const glm::vec3& value) const;
    const Shader& setVec3(const std::string& name, float x, float y, float z) const;
    const Shader& setVec4(const std::string& name, const glm::vec4& value) const;
    const Shader& setVec4(const std::string& name, float x, float y, float z, float w) const;
    const Shader& setMat2(const std::string& name, const glm::mat2& mat) const;
    const Shader& setMat3(const std::string& name, const glm::mat3& mat) const;
    const Shader& setMat4(const std::string& name, const glm::mat4& mat) const;

#if ENGINE_CXX_OVERLOADS
    const Shader& set(const std::string& name, bool value) const;
    const Shader& set(const std::string& name, int value) const;
    const Shader& set(const std::string& name, float value) const;
    const Shader& set(const std::string& name, const glm::vec2& value) const;
    const Shader& set(const std::string& name, float x, float y) const;
    const Shader& set(const std::string& name, const glm::vec3& value) const;
    const Shader& set(const std::string& name, float x, float y, float z) const;
    const Shader& set(const std::string& name, const glm::vec4& value) const;
    const Shader& set(const std::string& name, float x, float y, float z, float w) const;
    const Shader& set(const std::string& name, const glm::mat2& mat) const;
    const Shader& set(const std::string& name, const glm::mat3& mat) const;
    const Shader& set(const std::string& name, const glm::mat4& mat) const;
#endif
};