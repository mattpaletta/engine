#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <constants/shader.hpp>
#include <constants/texture.hpp>

#include "game_object.hpp"
#include "3d_renderer.hpp"
#include "mesh.hpp"

class Model final: public GameObject {
private:
	std::vector<Mesh> meshes;
	//bool gammaCorrection;
	
	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(Engine* engine, const aiNode& node, const aiScene& scene);
	Mesh processMesh(Engine* engine, const aiMesh& mesh, const aiScene& scene) const;

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	std::vector<Texture2D> loadMaterialTexture(Engine* engine, const aiMaterial& mat, const aiTextureType& type, const std::string& typeName) const;
	
	// Overrided version
	using GameObject::Draw;

public:
	std::string fragmentOutColour = "FragColour";
	std::string diffuseDesc = "texture_diffuse";
	std::string specularDesc = "texture_specular";
	std::string normalDesc = "texture_normal";
	std::string heightDesc = "texture_height";

	Model();
	~Model();


	Mesh& getMesh(const std::size_t i);
	std::size_t numMeshes() const;
	
	void loadModel(Engine* engine, const std::string& path);

	void Init();
	void UpdatePerspective(Renderer3D* renderer);
	void Draw(const glm::mat4& model) const;
};
