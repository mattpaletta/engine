#include "engine/model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

Model::Model(Engine* engine, const std::string& path) {
	this->loadModel(engine, path);
}

Model::~Model() {
	for (auto& mesh : this->meshes) {
		mesh.Cleanup();
	}
}

Mesh& Model::getMesh(const std::size_t& i) {
		return this->meshes.at(i);
}

std::size_t Model::numMeshes() const {
		return this->meshes.size();
}

void Model::Init(Engine* engine) {
    for (auto& mesh : this->meshes) {
        mesh.autoCreateShader(engine);
    }

    for (auto& mesh : this->meshes) {
		mesh.Init();
	}
}

void Model::UpdatePerspective(Engine* engine) {
    // Determine light count just once.
    const std::size_t currentLightCount = engine->getLightManager()->getLightCount();
	for (auto& mesh : this->meshes) {
        if (this->prevLightCount != currentLightCount) {
            // Regenerate the shader.
            mesh.autoCreateShader(engine);
        }
		mesh.UpdatePerspective(engine);
	}
    prevLightCount = currentLightCount;
#if ENGINE_DEBUG
    if (currentLightCount == 0) {
        std::cerr << "WARNING::MODEL::No lights in scene." << std::endl;
    }
#endif
}

void Model::Draw(const glm::mat4& model) const {
	for (const auto& mesh : this->meshes) {
		mesh.Draw(model);
	}
}

void Model::loadModel(Engine* engine, const std::string& path) {
	// read file via ASSIMP
	Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, /*aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace*/ aiProcessPreset_TargetRealtime_MaxQuality);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	// retrieve the directory path of the filepath
	// directory = path.substr(0, path.find_last_of('/'));
	const auto directory = constants::fs::path(path).parent_path();
	// process ASSIMP's root node recursively
	this->processNode(engine, *scene->mRootNode, *scene, directory);
}

void Model::processNode(Engine* engine, const aiNode& node, const aiScene& scene, const constants::fs::path& root_dir) {
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
		// the node object only contains indices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		const aiMesh* mesh = scene.mMeshes[node.mMeshes[i]];
		meshes.emplace_back(processMesh(engine, *mesh, scene, root_dir));
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node.mNumChildren; ++i) {
		this->processNode(engine, *node.mChildren[i], scene, root_dir);
	}
}

Mesh Model::processMesh(Engine* engine, const aiMesh& mesh, const aiScene& scene, const constants::fs::path& root_dir) {
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture2D> textures;

	auto ai_to_vec3 = [](const aiVector3D& inVector) {
		glm::vec3 vector;
		vector.x = inVector.x;
		vector.y = inVector.y;
		vector.z = inVector.z;
		return vector;
	};

	auto ai_to_vec2 = [](const aiVector3D& inVector) {
		glm::vec2 vector;
		vector.x = inVector.x;
		vector.y = inVector.y;
		return vector;
	};

	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh.mNumVertices; ++i) {
		Vertex vertex;
		vertex.Position = ai_to_vec3(mesh.mVertices[i]);
		vertex.Normal = ai_to_vec3(mesh.mNormals[i]);
		// texture coordinates
		if (mesh.mTextureCoords[0]) {// does the mesh contain texture coordinates?
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.TexCoords = ai_to_vec2(mesh.mTextureCoords[0][i]);
		} else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

        if (mesh.HasTangentsAndBitangents()) {
            vertex.Tangent = ai_to_vec3(mesh.mTangents[i]);
            vertex.Bitangent = ai_to_vec3(mesh.mBitangents[i]);
        } else {
            vertex.Tangent = glm::vec3(0);
            vertex.Bitangent = glm::vec3(0);
        }
        vertex.has_tangents = mesh.HasTangentsAndBitangents();
		vertices.emplace_back(std::move(vertex));
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
		const aiFace& face = mesh.mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices.emplace_back(face.mIndices[j]);
		}
	}

    auto ai_to_colour = [](const aiMaterial* material, const char* k, const unsigned int& i, const unsigned int& j) {
        aiColor3D matColour (0.f,0.f,0.f);
        material->Get(k, i, j, matColour);
        return glm::vec3(matColour.r, matColour.g, matColour.b);
    };

	// process materials
	const aiMaterial* material = scene.mMaterials[mesh.mMaterialIndex];
    Material mat;
    mat.DiffuseColour = ai_to_colour(material, AI_MATKEY_COLOR_DIFFUSE);
    mat.AmbientColour = ai_to_colour(material, AI_MATKEY_COLOR_AMBIENT);
    mat.SpecularColour = ai_to_colour(material, AI_MATKEY_COLOR_SPECULAR);
    mat.EmissiveColour = ai_to_colour(material, AI_MATKEY_COLOR_EMISSIVE);
    mat.TransparentColour = ai_to_colour(material, AI_MATKEY_COLOR_TRANSPARENT);
    
    material->Get(AI_MATKEY_SHININESS, mat.shininess);
    material->Get(AI_MATKEY_TEXBLEND(aiTextureType_DIFFUSE, 0), mat.diffuse_tex_blend);
    material->Get(AI_MATKEY_TEXBLEND(aiTextureType_SPECULAR, 0), mat.specular_tex_blend);
    material->Get(AI_MATKEY_TEXBLEND(aiTextureType_AMBIENT, 0), mat.ambient_tex_blend);

    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	std::vector<Texture2D> diffuseMaps = loadMaterialTextures(engine, *material, aiTextureType_DIFFUSE, "texture_diffuse", root_dir);
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 2. specular maps
	std::vector<Texture2D> specularMaps = loadMaterialTextures(engine, *material, aiTextureType_SPECULAR, "texture_specular", root_dir);
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 3. normal maps
	std::vector<Texture2D> normalMaps = loadMaterialTextures(engine, *material, aiTextureType_HEIGHT, "texture_normal", root_dir);
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 4. height maps
	std::vector<Texture2D> heightMaps = loadMaterialTextures(engine, *material, aiTextureType_AMBIENT, "texture_height", root_dir);
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	// return a mesh object created from the extracted mesh data
	Mesh finalMesh(vertices, indices, textures, mat);
	finalMesh.fragmentOutColour = this->fragmentOutColour;
	finalMesh.diffuseDesc = this->diffuseDesc;
	finalMesh.specularDesc = this->specularDesc;
	finalMesh.normalDesc = this->normalDesc;
	finalMesh.heightDesc = this->heightDesc;
	return finalMesh;
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<Texture2D> Model::loadMaterialTextures(Engine* engine, const aiMaterial& mat, const aiTextureType& type, const std::string& typeName, const constants::fs::path& root_dir) {
	std::vector<Texture2D> materialTextures;
	for (unsigned int i = 0; i < mat.GetTextureCount(type); ++i) {
		aiString str;
		mat.GetTexture(type, i, &str);

		// The texture name is just the path of the file
		const std::string tex_path = root_dir.string() + "/" + std::string(str.C_Str());
		const std::string tex_name = "model_" + tex_path;
		auto* resourceManager = engine->getResourceManager();
		// Return the old texture if it is already loaded.
		Texture2D texture = resourceManager->TextureLoaded(tex_name) ?
			resourceManager->GetTexture(tex_name) :
			resourceManager->LoadTexture(tex_path, tex_name, false, true); // Don't flip, generate mipmap.

		texture.desc = typeName;
		materialTextures.push_back(texture);
	}

	return materialTextures;
}
