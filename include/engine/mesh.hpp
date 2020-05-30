#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <constants/texture.hpp>
#include <constants/shader.hpp>
#include "vertex.hpp"
#include "3d_renderer.hpp"

#include <string>
#include <vector>
#include <iostream>

/*
struct Texture {
    unsigned int id;
	std::string type;
	std::string path;
};*/

class Mesh {
public:
	std::string fragmentOutColour;
	std::string diffuseDesc;
	std::string specularDesc;
	std::string normalDesc;
	std::string heightDesc;

    Mesh(const std::vector<Vertex>& _vertices, const std::vector<unsigned int>& _indices, const std::vector<Texture2D>& _textures);
	~Mesh();

	// Not copyable
	Mesh(const Mesh& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;

	Mesh(Mesh&& other)  = default;
	Mesh& operator=(Mesh&& other) = default;

	void Init();
	void Cleanup();
    void UpdatePerspective(Renderer3D* renderer);
	void Draw(const glm::mat4& model) const;

	bool autoCreateShader(const std::vector<std::string>& fragmentCode);

private:
	mutable Shader shader;

	std::string create_vertex_shader() const;
	std::string create_fragment_shader(const std::vector<std::string>& fragmentCode) const;

	// The number of each texture, to bind to shader
	unsigned int diffuseNr = 0;
	unsigned int specularNr = 0;
	unsigned int normalNr = 0;
	unsigned int heightNr = 0;

	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture2D>      textures;

    unsigned int VAO;
    unsigned int VBO;
	unsigned int EBO;
};
