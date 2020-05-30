#include "engine/mesh.hpp"

unsigned int countNumTextureType(const std::vector<Texture2D>& textures, const std::string& texType) {
	unsigned int count = 0;
	for (const auto& tex : textures) {
		if (tex.desc == texType) {
			count += 1;
		}
	}

	return count;
}

const std::string opengl_version = "#version 330 core\n";
const std::string texture_import_name = "aTexCoords";
const std::string texture_pass_name = "TexCoords";

std::string Mesh::create_vertex_shader() const {
    const std::string texture_import = !this->use_textures ? "" : "layout(location = 2) in vec2 "+texture_import_name+";\n";
    const std::string texture_export = !this->use_textures ? "" : "out vec2 "+texture_pass_name+";\n";
    const std::string texture_pass = !this->use_textures ? "" : texture_pass_name+" = "+texture_import_name+";\n";

    std::string shader_code =
        opengl_version +
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec3 aNormal;\n"
        ""+texture_import+"\n" // This is conditional based on (this->use_textures)
        "\n"
        "out vec3 FragPos;\n"
        "out vec3 Normal;\n"
        ""+texture_export+"\n" // This is conditional based on (this->use_textures)
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main() {\n"
        "   "+texture_pass+"\n" // This is conditional based on (this->use_textures)
        "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";
    return shader_code;
}

std::string Mesh::create_fragment_shader() const {
    const std::string texture_import = !this->use_textures ? "" : "in vec2 "+texture_pass_name+";\n";

    std::string texture_shaders = "";
    if (this->use_textures) {
        for (unsigned int i = 1; i <= this->diffuseNr; ++i) {
            texture_shaders += "uniform sampler2D " + this->diffuseDesc + std::to_string(i) + ";\n";
        }
        for (unsigned int i = 1; i <= this->specularNr; ++i) {
            texture_shaders += "uniform sampler2D " + this->specularDesc + std::to_string(i) + ";\n";
        }
        for (unsigned int i = 1; i <= this->normalNr; ++i) {
            texture_shaders += "uniform sampler2D " + this->normalDesc + std::to_string(i) + ";\n";
        }
        for (unsigned int i = 1; i <= this->heightNr; ++i) {
            texture_shaders += "uniform sampler2D " + this->heightDesc + std::to_string(i) + ";\n";
        }
    }
    const std::string material_struct =
        "struct Material {\n"
        "   vec3 ambient;\n"
        "   vec3 diffuse;\n"
        "   vec3 specular;\n"
        "\n"
        "   float shininess;\n"
        "\n"
        "   float ambientMix;\n"
        "   float diffuseMix;\n"
        "   float specularMix;\n"
        "};\n";
    const std::string light_struct =
        "struct Light {\n"
        "    vec3 position;\n"
        "    vec3 ambient;\n"
        "    vec3 diffuse;\n"
        "    vec3 specular;\n"
        "};\n";

    const std::string shader_begin =
        opengl_version +
        material_struct +
        light_struct +
        "out vec4 "+this->fragmentOutColour+";\n"
        "\n"
        "in vec3 FragPos;\n"
        "in vec3 Normal;\n"
        "\n"
        ""+texture_import+"\n"
        "\n"
        "uniform vec3 viewPos;\n"
        "uniform Material material;\n"
        "uniform Light light;\n"
        "\n";
    // TODO: Combine texture w ambient colour.

    // ambient
    const std::string calculate_ambient =
        "   vec3 ambient = light.ambient * material.ambient;\n";

    const std::string texture_diffuse = this->use_textures ?
        "mix(material.diffuse, texture(texture_diffuse1, "+texture_pass_name+").xyz, material.diffuseMix)" :
        "material.diffuse";
    const std::string calculate_diffuse =
        "   vec3 norm = normalize(Normal);\n"
        "   vec3 lightDir = normalize(light.position - FragPos);\n"
        "   float diff = max(dot(norm, lightDir), 0.0);\n"
        "   vec3 diffuse = light.diffuse * (diff * "+texture_diffuse+");\n";

    // specular
    const std::string texture_specular = this->use_textures ?
        "mix(material.specular, texture(texture_specular1, "+texture_pass_name+").xyz, material.specularMix)" :
        "material.specular";
    const std::string calculate_specular =
        "   vec3 viewDir = normalize(viewPos - FragPos);\n"
        "   vec3 reflectDir = reflect(-lightDir, norm);\n"
        "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
        "   vec3 specular = light.specular * (spec * "+texture_specular+");\n";

    const std::string calculate_final_lighting =
        "   vec3 result = ambient + diffuse + specular;\n"
        "   "+this->fragmentOutColour+" = vec4(result, 1.0);";

    const std::string shader_main =
        "void main() {\n"
        ""+calculate_ambient+"\n"
        ""+calculate_diffuse+"\n"
        ""+calculate_specular+"\n"
        ""+calculate_final_lighting+"\n"
        "}\n";
    return
        shader_begin+
        texture_shaders+
        shader_main;
}

Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<unsigned int>& _indices, const std::vector<Texture2D>& _textures, const Material& _material) :  use_textures(_textures.size() > 0), vertices(_vertices), indices(_indices), textures(_textures), material(_material) {}

Mesh::~Mesh() {
	// Don't cleanup in here, because the mesh is moved as it is being managed by model.
	// Model will call Cleanup before being deallocated.
}

void Mesh::Cleanup() {
	glDeleteVertexArrays(1, &this->VAO);
	glDeleteBuffers(1, &this->VBO);
	glDeleteBuffers(1, &this->EBO);
}

bool Mesh::autoCreateShader() {
	// Recount texture types.
	this->diffuseNr = countNumTextureType(this->textures, this->diffuseDesc);
	this->specularNr = countNumTextureType(this->textures, this->specularDesc);
	this->normalNr = countNumTextureType(this->textures, this->normalDesc);
	this->heightNr = countNumTextureType(this->textures, this->heightDesc);

	const std::string vertex_code = this->create_vertex_shader();
	const std::string fragment_code = this->create_fragment_shader();
	this->shader = Shader(vertex_code, fragment_code);
#if ENGINE_DEBUG
	const bool is_valid = this->shader.valid();
	if (is_valid) {
		std::cout << "Shader compiled successfully" << std::endl;
	} else {
		std::cout << "Computed Shader::VERTEX --" << "\n" << vertex_code << "\n -- END VERTEX" << std::endl;
		std::cout << "Computed Shader::FRAGMENT --" << "\n" << fragment_code << "\n -- END FRAGMENT" << std::endl;
	}
	return is_valid;
#else
	return this->shader.valid();
#endif
}

std::string Mesh::description() const {
    return "Num Diffuse: (" + this->diffuseDesc + ") - " + std::to_string(this->diffuseNr) + "\n" +
        "Num Specular: (" + this->specularDesc + ") - " + std::to_string(this->specularNr) + "\n" +
        "Num Normal: (" + this->normalDesc + ") - " + std::to_string(this->normalNr) + "\n" +
        "Num Height: (" + this->heightDesc + ") - " + std::to_string(this->heightNr) + "\n";
}

void Mesh::UpdatePerspective(Renderer3D* renderer) {
	this->shader\
		.use()\
		.setMat4("projection", renderer->getProjection())\
        .setMat4("view", renderer->getView());

    // TODO: pass lighting & camera into mesh.
    const glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    this->shader\
        .setVec3("light.position", lightPos)\
        .setVec3("viewPos", renderer->getCameraPos());
}

// render the mesh
void Mesh::Draw(const glm::mat4& model) const {
	this->shader.use().setMat4("model", model);

//    glm::vec3 lightColor;
//    lightColor.x = sin(0 * 2.0f);
//    lightColor.y = sin(0 * 0.7f);
//    lightColor.z = sin(0 * 1.3f);
//    glm::vec3 diffuseColor = lightColor   * glm::vec3(1.0f); /*glm::vec3(0.5f)*/; // decrease the influence
//    glm::vec3 ambientColor = diffuseColor * glm::vec3(1.0f); // low influence
    this->shader.setVec3("light.ambient", glm::vec3(0.2f));
    this->shader.setVec3("light.diffuse", glm::vec3(0.8f));
    this->shader.setVec3("light.specular", glm::vec3(1.0f));

    // material properties
    this->shader.setVec3("material.ambient", this->material.AmbientColour);
    this->shader.setVec3("material.diffuse", this->material.DiffuseColour);
    this->shader.setVec3("material.specular", this->material.SpecularColour); // specular lighting doesn't have full effect on this object's material
    this->shader.setFloat("material.shininess", this->material.shininess);

    this->shader.setFloat("material.ambientMix", 1-this->material.ambient_tex_blend);
    this->shader.setFloat("material.diffuseMix", 1-this->material.diffuse_tex_blend);
    this->shader.setFloat("material.specularMix", 1-this->material.specular_tex_blend);

    if (this->use_textures) {
        // bind appropriate textures
        unsigned int diffuseIndex = 1;
        unsigned int specularIndex = 1;
        unsigned int normalIndex = 1;
        unsigned int heightIndex = 1;

        for (unsigned int i = 0; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                                              // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = textures[i].desc;
            if (name == this->diffuseDesc) {
                number = std::to_string(diffuseIndex++);
            } else if(name == this->specularDesc) {
                number = std::to_string(specularIndex++);
            } else if(name == this->normalDesc) {
                number = std::to_string(normalIndex++);
            } else if(name == this->heightDesc) {
                number = std::to_string(heightIndex++);
            }

            this->shader.setInt(name + number, static_cast<int>(i));
            this->textures[i].Bind();
        }
    }

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}


void Mesh::Init() {
	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	// create buffers/arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
    GLuint i = 0;
	// vertex Positions
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(i++, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
	// vertex normals
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(i++, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Normal));

    if (this->use_textures) {
        // vertex texture coords
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i++, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i++, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i++, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Bitangent));
    } else {
        // Everything else is a uniform.
    }
	glBindVertexArray(0);
}
