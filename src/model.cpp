#include "engine/model.hpp"
#include "engine/vertex.hpp"
#include "engine/engine.hpp"

#include <constants/filesystem.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <memory>

Model::Model() {}
Model::~Model() {}

void Model::loadModel(Engine* engine, const std::string& path) {
	// read file via Assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	const auto directory = constants::fs::path(path).parent_path();
	this->processNode(engine, *scene->mRootNode, *scene, directory);
}

Mesh& Model::getMesh(const std::size_t i) {
	return this->meshes.at(i);
}

std::size_t Model::numMeshes() const {
	return this->meshes.size();
}

void Model::UpdatePerspective(Renderer3D* renderer) {
	for (auto& mesh : this->meshes) {
		mesh.UpdatePerspective(renderer);
	}
}

void Model::Init() {
	for (auto& mesh : this->meshes) {
		mesh.Init();
	}
}

void Model::processNode(Engine* engine, const aiNode& node, const aiScene& scene, const constants::fs::path& root_dir) {
	for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
		const auto* mesh = scene.mMeshes[node.mMeshes[i]];
		this->meshes.emplace_back(this->processMesh(engine, *mesh, scene, root_dir));
	}

	for (unsigned int i = 0; i < node.mNumChildren; ++i) {
		this->processNode(engine, *node.mChildren[i], scene, root_dir);
	}
}

Mesh Model::processMesh(Engine* engine, const aiMesh& mesh, const aiScene& scene, const constants::fs::path& root_dir) const {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture2D> meshTextures;

	auto ai_to_glm3 = [](const aiVector3D& inVector) {
		glm::vec3 vector;
		vector.x = inVector.x;
		vector.y = inVector.y;
		vector.z = inVector.z;
		return vector;
	};
	auto ai_to_glm2 = [](const aiVector3D& inVector) {
		glm::vec2 vector;
		vector.x = inVector.x;
		vector.y = inVector.y;
		return vector;
	};

	// Walk model vertices
	for (unsigned int i = 0; i < mesh.mNumVertices; ++i) {
		Vertex vertex;

		vertex.Position = ai_to_glm3(mesh.mVertices[i]);
		vertex.Normal = ai_to_glm3(mesh.mNormals[i]);

		// texture coordinates
		if (mesh.mTextureCoords[0]) {// does the mesh contain texture coordinates?
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.TexCoords = ai_to_glm2(mesh.mTextureCoords[0][i]);
		} else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		vertex.Tangent = ai_to_glm3(mesh.mTangents[i]);
		vertex.Bitangent = ai_to_glm3(mesh.mBitangents[i]);
		vertices.emplace_back(std::move(vertex));
	}

	for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
		const aiFace face = mesh.mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Process Material
	const aiMaterial* material = scene.mMaterials[mesh.mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	std::vector<Texture2D> diffuseMaps = this->loadMaterialTexture(engine, *material, aiTextureType_DIFFUSE, "texture_diffuse", root_dir);
	meshTextures.insert(meshTextures.end(), diffuseMaps.begin(), diffuseMaps.end());

	// 2. specular maps
	std::vector<Texture2D> specularMaps = this->loadMaterialTexture(engine, *material, aiTextureType_SPECULAR, "texture_specular", root_dir);
	meshTextures.insert(meshTextures.end(), specularMaps.begin(), specularMaps.end());
	
	// 3. normal maps
	std::vector<Texture2D> normalMaps = this->loadMaterialTexture(engine, *material, aiTextureType_HEIGHT, "texture_normal", root_dir);
	meshTextures.insert(meshTextures.end(), normalMaps.begin(), normalMaps.end());
	
	// 4. height maps
	std::vector<Texture2D> heightMaps = this->loadMaterialTexture(engine, *material, aiTextureType_AMBIENT, "texture_height", root_dir);
	meshTextures.insert(meshTextures.end(), heightMaps.begin(), heightMaps.end());

	Mesh finalMesh(vertices, indices, meshTextures);
	finalMesh.diffuseDesc = this->diffuseDesc;
	finalMesh.specularDesc = this->specularDesc;
	finalMesh.normalDesc = this->normalDesc;
	finalMesh.heightDesc = this->heightDesc;
	finalMesh.fragmentOutColour = this->fragmentOutColour;
	return finalMesh;
}

std::vector<Texture2D> Model::loadMaterialTexture(Engine* engine, const aiMaterial& mat, const aiTextureType& type, const std::string& typeName, const constants::fs::path& root_dir) const {
	std::vector<Texture2D> materialTextures;
	for (unsigned int i = 0; i < mat.GetTextureCount(type); ++i) {
		aiString str;
		mat.GetTexture(type, i, &str);

        const std::string model_path = root_dir.string() + "/" + std::string(str.C_Str());
        const std::string model_name = "model_" + model_path;
		auto texture = engine->getResourceManager()->LoadTexture(model_path, model_name);
		texture.desc = typeName;
		// This will automatically return the old texture if it is already loaded.
		materialTextures.push_back(texture);
	}

	return materialTextures;
}

void Model::Draw(const glm::mat4& model) const {
	for (const auto& mesh : this->meshes) {
		mesh.Draw(model);
	}
}
