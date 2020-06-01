#pragma once

#include <assimp/scene.h>

#include <constants/filesystem.hpp>

#include "engine.hpp"
#include "mesh.hpp"
#include "game_object.hpp"
#include "3d_renderer.hpp"

class Model final : public GameObject {
public:
	std::string fragmentOutColour = "FragColour";
	std::string diffuseDesc = "texture_diffuse";
	std::string specularDesc = "texture_specular";
	std::string normalDesc = "texture_normal";
	std::string heightDesc = "texture_height";

    // constructor, expects a filepath to a 3D model.
    Model(Engine* engine, const std::string& path);
	~Model();

	Mesh& getMesh(const std::size_t& i);
	std::size_t numMeshes() const;
	void Init(Engine* engine);
	using GameObject::Draw;
	void UpdatePerspective(Engine* engine);
    void Draw(const glm::mat4& model) const;

private:
	std::vector<Mesh> meshes;
//    const bool gammaCorrection;
    std::size_t prevLightCount = 0;

    void loadModel(Engine* engine, const std::string& path);
    void processNode(Engine* engine, const aiNode& node, const aiScene& scene, const constants::fs::path& root_dir);
    Mesh processMesh(Engine* engine, const aiMesh& mesh, const aiScene& scene, const constants::fs::path& root_dir);
	std::vector<Texture2D> loadMaterialTextures(Engine* engine, const aiMaterial& mat, const aiTextureType& type, const std::string& typeName, const constants::fs::path& root_dir);
};
