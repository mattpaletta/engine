#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <iostream>

#include <constants/texture.hpp>
#include <constants/shader.hpp>

#include "vertex.hpp"
#include "material.hpp"
#include "3d_renderer.hpp"
#include "engine_fwd.hpp"
#include "model_fwd.hpp"

class Mesh {
public:
    friend Model;

	std::string fragmentOutColour;
	std::string diffuseDesc;
	std::string specularDesc;
	std::string normalDesc;
	std::string heightDesc;

    Mesh(const std::vector<Vertex>& _vertices, const std::vector<unsigned int>& _indices, const std::vector<Texture2D>& _textures, const Material& _material);
	~Mesh();

	// Not copyable
	Mesh(const Mesh& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;

	Mesh(Mesh&& other)  = default;
	Mesh& operator=(Mesh&& other) = default;

    std::string description() const;

private:
	mutable Shader shader;
    bool use_textures;

    // only accessible by Model.
    bool autoCreateShader(Engine* engine);
    void Cleanup();
    void Init();
    void UpdatePerspective(Engine* engine);
    void Draw(const glm::mat4& model) const;

	std::string create_vertex_shader() const;
	std::string create_fragment_shader(const std::size_t& numDirLights, const std::size_t& numPointLights, const std::size_t& numSpotlights) const;

	// The number of each texture type, used to generate shader
	unsigned int diffuseNr = 0;
	unsigned int specularNr = 0;
	unsigned int normalNr = 0;
	unsigned int heightNr = 0;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture2D> textures;
    Material material;


    // OpenGL Contexts
    unsigned int VAO;
    unsigned int VBO;
	unsigned int EBO;
};
