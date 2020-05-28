#pragma once
#include <vector>
#include <constants/texture.hpp>
#include <constants/shader.hpp>

#include "3d_renderer.hpp"
#include "vertex.hpp"

class Mesh {
private:
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	const std::vector<Vertex> vertices;
	const std::vector<unsigned int> indices;
	const std::vector<Texture2D> textures;

	// The number of each texture, to bind to shader
	unsigned int diffuseNr = 0;
	unsigned int specularNr = 0;
	unsigned int normalNr = 0;
	unsigned int heightNr = 0;

	mutable Shader shader;

	std::string create_vertex_shader() const;
	std::string create_fragment_shader(const std::vector<std::string>& fragmentCode) const;

public:
	std::string fragmentOutColour = "FragColour";
	std::string diffuseDesc = "texture_diffuse";
	std::string specularDesc = "texture_specular";
	std::string normalDesc = "texture_normal";
	std::string heightDesc = "texture_height";

	Mesh(const std::vector<Vertex>& _vertices, const std::vector<unsigned int>& _indices, const std::vector<Texture2D>& _textures);
	~Mesh();

	Mesh& operator=(const Mesh&) = delete;

	// Getters
	unsigned int getNumDiffuse() const { return this->diffuseNr; }
	unsigned int getNumSpecular() const { return this->specularNr; }
	unsigned int getNumNormal() const { return this->normalNr; }
	unsigned int getNumHeight() const { return this->heightNr; }

	void setShader(const Shader& _shader);
	bool autoCreateShader(const std::vector<std::string>& fragmentCode);
	
	void Init();
	void UpdatePerspective(Renderer3D* renderer);
	void Draw(const glm::mat4& model) const;
};