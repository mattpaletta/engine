#include "engine/skybox.hpp"
#include "engine/engine.hpp"

#include <array>

Skybox::Skybox() : VAO(0), VBO(0) {}
Skybox::~Skybox() {}

void Skybox::Init(Engine* engine, const CubeMap& cubemap) {

    const std::string skybox_vert =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "out vec2 TexCoords;\n"
        "\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main() {\n"
        "	TexCoords = aPos;\n"
        "   vec4 pos = projection * view * vec4(aPos, 1.0);"
        "	gl_Position = pos.xyww;\n"
        "}";
    const std::string skybox_frag =
        "#version 330 core\n"
        "out vec4 FragColour;\n"
        "\n"
        "in vec3 TexCoords;\n"
        "\n"
        "uniform samplerCube skybox;\n"
        "\n"
        "void main() {\n"
        "   FragColour = texture(skybox, TexCoords);\n"
        "}";

    constexpr std::array<float, 3 * 6 * 6> skyboxVertices{ {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    }};

    // skybox VAO
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), skyboxVertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

    this->texture = engine->getResourceManager()->LoadCubeMap(cubemap, false, "skybox");
    engine->getResourceManager()->SetTextureAsSelfUsed("skybox");
    
    this->shader = Shader(skybox_vert, skybox_frag);
    this->shader\
        .use()\
        .setInt("skybox", 0);
}

void Skybox::Draw(Renderer3D* renderer) const noexcept {
    // draw skybox as last
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

    this->shader.use();
    const auto view = glm::mat4(glm::mat3(renderer->getView())); // remove translation from the view matrix
    this->shader.setMat4("view", view);
    this->shader.setMat4("projection", renderer->getProjection());
    // skybox cube
    glBindVertexArray(this->VAO);
    glActiveTexture(GL_TEXTURE0);
    this->texture.BindCubeMap();

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
}