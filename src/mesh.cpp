#include "engine/mesh.hpp"
#include "engine/debug.hpp"

#include <string>

unsigned int countNumTextureType(const std::vector<Texture2D>& textures, const std::string& texType) {
    unsigned int count = 0;
    for (const auto& tex : textures) {
        if (tex.desc == texType) {
            count += 1;
        }
    }

    return count;
}

std::string Mesh::create_vertex_shader() const {
    return
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec3 aNormal;\n"
        "layout(location = 2) in vec2 aTexCoords;\n"
        "\n"
        "out vec2 TexCoords;\n"
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main() {\n"
        "    TexCoords = aTexCoords;\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\n";
}

std::string Mesh::create_fragment_shader(const std::vector<std::string>& fragmentCode) const {
    std::string shader_begin =
        "#version 330 core\n"
        "out vec4 "+this->fragmentOutColour+";\n"
        "\n"
        "in vec2 TexCoords;\n"
        "\n";

    std::string texture_shaders = "";
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
    std::string shader_end =
        "\n"
        "void main() {\n";
    for (const auto& line : fragmentCode) {
        shader_end += (line + "\n");
    }
    shader_end += "}\n";
    return shader_begin + texture_shaders + shader_end;
}

Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<unsigned int>& _indices, const std::vector<Texture2D>& _textures) :
    vertices(_vertices),
    indices(_indices),
    textures(_textures) {}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->EBO);
}

void Mesh::setShader(const Shader& _shader) {
    this->shader = _shader;
}

bool Mesh::autoCreateShader(const std::vector<std::string>& fragmentCode) {
    // Recount texture types.
    this->diffuseNr = countNumTextureType(this->textures, this->diffuseDesc);
    this->specularNr = countNumTextureType(this->textures, this->specularDesc);
    this->normalNr = countNumTextureType(this->textures, this->normalDesc);
    this->heightNr = countNumTextureType(this->textures, this->heightDesc);

    const std::string vertex_code = this->create_vertex_shader();
    const std::string fragment_code = this->create_fragment_shader(fragmentCode);
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

void Mesh::Init() {
    // Recount texture types.
    this->diffuseNr = countNumTextureType(this->textures, this->diffuseDesc);
    this->specularNr = countNumTextureType(this->textures, this->specularDesc);
    this->normalNr = countNumTextureType(this->textures, this->normalDesc);
    this->heightNr = countNumTextureType(this->textures, this->heightDesc);

    // create buffers/arrays
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(VAO);

    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, static_cast<int>(this->vertices.size() * sizeof(Vertex)), this->vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<int>(this->indices.size() * sizeof(unsigned int)), this->indices.data(), GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal)));

    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));

    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Tangent)));

    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Bitangent)));

    glBindVertexArray(0);

    unsigned int diffuseIndex = 0;
    unsigned int specularIndex = 0;
    unsigned int normalIndex = 0;
    unsigned int heightIndex = 0;

    // Setup the textures
    for (unsigned int i = 0; i < this->textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                                          // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].desc;
        if (name == this->diffuseDesc) {
            number = std::to_string(diffuseIndex++);
        } else if (name == this->specularDesc) {
            number = std::to_string(specularIndex++);
        } else if (name == this->normalDesc) {
            number = std::to_string(normalIndex++);
        } else if (name == this->heightDesc) {
            number = std::to_string(heightIndex++);
        }

        // now set the sampler to the correct texture unit
        this->shader.use().setInt(name + number, static_cast<int>(i));
        this->textures[i].Bind();
    }
}

void Mesh::UpdatePerspective(Renderer3D* renderer) {
    this->shader\
        .use()\
        .setMat4("projection", renderer->getProjection())\
        .setMat4("view", renderer->getView());
    glCheckError();
}

void Mesh::Draw(const glm::mat4& model) const {
    this->shader\
        .use()\
        .setMat4("model", model);
    glCheckError();

    // draw mesh
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<int>(this->indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}
