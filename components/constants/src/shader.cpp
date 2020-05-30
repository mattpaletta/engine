#include "shader.hpp"
#include "filesystem.hpp"

#include "glad/glad.h" // include glad to get all the required OpenGL headers

#include <fstream>
#include <sstream>
#include <iostream>

namespace {
    unsigned int CompileShader(const std::string& shaderSource, const GLuint& shaderType) {
        unsigned int shader;
        const auto raw_str = shaderSource.c_str();
        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &raw_str, NULL);
        glCompileShader(shader);
        return shader;
    }

    bool VerifyShader(unsigned int shader, [[maybe_unused]] const std::string& step) {
        int  success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

#if ENGINE_DEBUG
        constexpr std::size_t info_msg_length = 512;
        char infoLog[info_msg_length];
        if (!success) {
            glGetShaderInfoLog(shader, info_msg_length, NULL, infoLog);
            std::cerr << "ERROR::SHADER::" << step << "::COMPILATION_FAILED - " << infoLog << std::endl;
        }
#endif
        return success;
    }

    unsigned int CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
        unsigned int vertexShader = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
#if ENGINE_DEBUG
        if (!VerifyShader(vertexShader, "VERTEX")) {
            std::cerr << "Failed to compile vertex shader" << std::endl;
        }
#endif
        unsigned int fragmentShader = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
#if ENGINE_DEBUG
        if (!VerifyShader(fragmentShader, "FRAGMENT")) {
            // Free Memory
            std::cerr << "Failed to compiile fragment shader" << std::endl;
            glDeleteShader(vertexShader);
        }
#endif

        unsigned int shaderProgram;
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return shaderProgram;
    }

    bool VerifyShaderProgram(unsigned int shaderProgram) {
        int success;
        constexpr std::size_t info_msg_length = 512;

        char infoLog[info_msg_length];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, info_msg_length, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED - " << infoLog << std::endl;
        }
        return success;
    }
}

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) : ID(CreateShaderProgram(vertexSrc, fragmentSrc)) {}

Shader Shader::from_file(const std::string& vertexPath, const std::string& fragmentPath) {
    // Helper function that reads the source code from each path, along with error messages.
    auto readFile = [](const std::string& inputFile, const std::string& shaderType) {
        std::string shaderCode = "";
        if (inputFile == "") {
            return shaderCode;
        } else if (!constants::fs::exists(inputFile)) {
            std::cerr << "ERROR::" << shaderType << "::FILE_NOT_FOUND - " << inputFile << std::endl;
            return shaderCode;
        }

        std::ifstream shaderFile;

        // ensure ifstream objects can throw exceptions:
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open files
            shaderFile.open(inputFile);
            std::stringstream shaderStream;

            // read file's buffer contents into streams
            shaderStream << shaderFile.rdbuf();

            // close file handlers
            shaderFile.close();

            // convert stream into string
            shaderCode = shaderStream.str();
        } catch (const std::ifstream::failure& e) {
            std::cerr << "ERROR::" << shaderType << "::FILE_NOT_SUCCESFULLY_READ (" << e.what() << ") - " << inputFile << std::endl;
        }

        return shaderCode;
    };

    return { readFile(vertexPath, "VERTEX"), readFile(fragmentPath, "FRAGMENT") };
}

unsigned int Shader::id() const {
    return this->ID;
}

bool Shader::valid() const {
    return this->ID > 0 && VerifyShaderProgram(this->ID);
}

Shader& Shader::use() {
#if ENGINE_DEBUG
    if (this->ID == 0) {
        std::cout << "Warning: Shader ID 0" << std::endl;
    }
#endif
    glUseProgram(this->ID);
    return *this;
}

const Shader& Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(this->ID, name.c_str()), static_cast<int>(value));
    return *this;
}

const Shader& Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value);
    return *this;
}

const Shader& Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
    return *this;
}

const Shader& Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    return *this;
}

const Shader& Shader::setVec2(const std::string& name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    return *this;
}

const Shader& Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    return *this;
}

const Shader& Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    return *this;
}

const Shader& Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    return *this;
}

const Shader& Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    return *this;
}

const Shader& Shader::setMat2(const std::string& name, const glm::mat2& mat) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    return *this;
}

const Shader& Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    return *this;
}

const Shader& Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    return *this;
}

#if ENGINE_CXX_OVERLOADS
const Shader& Shader::set(const std::string& name, bool value) const {
    return this->setBool(name, value);
}

const Shader& Shader::set(const std::string& name, int value) const {
    return this->setInt(name, value);
}

const Shader& Shader::set(const std::string& name, float value) const {
    return this->setFloat(name, value);
}

const Shader& Shader::set(const std::string& name, const glm::vec2& value) const {
    return this->setVec2(name, value);
}

const Shader& Shader::set(const std::string& name, float x, float y) const {
    return this->setVec2(name, x, y);
}

const Shader& Shader::set(const std::string& name, const glm::vec3& value) const {
    return this->setVec3(name, value);
}

const Shader& Shader::set(const std::string& name, float x, float y, float z) const {
    return this->setVec3(name, x, y, z);
}

const Shader& Shader::set(const std::string& name, const glm::vec4& value) const {
    return this->setVec4(name, value);
}

const Shader& Shader::set(const std::string& name, float x, float y, float z, float w) const {
    return this->setVec4(name, x, y, z, w);
}

const Shader& Shader::set(const std::string& name, const glm::mat2& mat) const {
    return this->setMat2(name, mat);
}

const Shader& Shader::set(const std::string& name, const glm::mat3& mat) const {
    return this->setMat3(name, mat);
}

const Shader& Shader::set(const std::string& name, const glm::mat4& mat) const {
    return this->setMat4(name, mat);
}
#endif
