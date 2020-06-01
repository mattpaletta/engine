#include "engine/mesh.hpp"
#include "engine/engine.hpp"

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

std::string Mesh::create_fragment_shader(const std::size_t& numDirLights, const std::size_t& numPointLights, const std::size_t& numSpotLights) const {
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
    const std::string texture_diffuse = this->use_textures ?
        "mix(material.diffuse, texture(texture_diffuse1, "+texture_pass_name+").xyz, material.diffuseMix)" :
        "material.diffuse";
    const std::string texture_specular = this->use_textures ?
        "mix(material.specular, texture(texture_specular1, "+texture_pass_name+").xyz, material.specularMix)" :
        "material.specular";

    const std::string calcDirLight_def = "vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)";
    const std::string calcDirLight_fwd = calcDirLight_def+";\n";
    const auto calcDirLight = [&calcDirLight_def](const std::string& texture_diffuse, const std::string& texture_specular) {
        return "" + calcDirLight_def + " {\n"
            "   vec3 lightDir = normalize(-light.direction);\n"
            "   // diffuse shading\n"
            "   float diff = max(dot(normal, lightDir), 0.0);\n"
            "   // specular shading\n"
            "   vec3 reflectDir = reflect(-lightDir, normal);\n"
            "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
            "   // combine results\n"
            "   vec3 ambient = light.ambient * "+texture_diffuse+";\n"
            "   vec3 diffuse = light.diffuse * (diff * "+texture_diffuse+");\n"
            "   vec3 specular = light.specular * (spec * "+texture_specular+");\n"
            "   return (ambient + diffuse + specular);\n"
            "}\n";
    };

    const std::string calcPointLight_def = "vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)";
    const std::string calcPointLight_fwd = calcPointLight_def+";\n";
    const auto calcPointLight = [&calcPointLight_def](const std::string& texture_diffuse, const std::string& texture_specular) {
        return
            ""+calcPointLight_def+" {\n"
            "   vec3 lightDir = normalize(light.position - fragPos);\n"
            "   // diffuse shading\n"
            "   float diff = max(dot(normal, lightDir), 0.0);"
            "   // specular shading\n"
            "   vec3 reflectDir = reflect(-lightDir, normal);\n"
            "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
            "   // attenuation\n"
            "   float distance = length(light.position - fragPos);\n"
            "   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
            "   // combine results\n"
            "   vec3 ambient = light.ambient * "+texture_diffuse+";\n"
            "   vec3 diffuse = light.diffuse * (diff * "+texture_diffuse+");\n"
            "   vec3 specular = light.specular * (spec * "+texture_specular+");\n"
            "   return (ambient + diffuse + specular) * attenuation;\n"
            "}\n";
    };

    const std::string calcSpotLight_def = "vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)";
    const std::string calcSpotLight_fwd = calcSpotLight_def+";\n";
    const auto calcSpotLight = [&calcSpotLight_def](const std::string& texture_diffuse, const std::string& texture_specular) {
        return
            ""+calcSpotLight_def+" {\n"
            "   vec3 lightDir = normalize(light.position - fragPos);\n"
            "   // diffuse shading\n"
            "   float diff = max(dot(normal, lightDir), 0.0);\n"
            "   // specular shading\n"
            "   vec3 reflectDir = reflect(-lightDir, normal);\n"
            "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
            "   // attenuation\n"
            "   float distance = length(light.position - fragPos);\n"
            "   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
            "   // spotlight intensity\n"
            "   float theta = dot(lightDir, normalize(-light.direction));\n"
            "   float epsilon = light.cutOff - light.outerCutOff;\n"
            "   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n"
            "   // combine results\n"
            "   vec3 ambient = light.ambient * "+texture_diffuse+";\n"
            "   vec3 diffuse = light.diffuse * diff * "+texture_diffuse+";\n"
            "   vec3 specular = light.specular * spec * "+texture_specular+";\n"
            "   return (ambient + diffuse + specular) * attenuation * intensity;\n"
            "}\n";
    };

    // DEFINE LIGHTING & MATERIAL STRUCTS
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
    const std::string direction_light_struct =
        "struct DirLight {\n"
        "   vec3 direction;\n"
        "\n"
        "   vec3 ambient;\n"
        "   vec3 diffuse;\n"
        "   vec3 specular;\n"
        "};\n";
    const std::string point_light_struct =
        "struct PointLight {\n"
        "   vec3 position;\n"
        "\n"
        "   float constant;\n"
        "   float linear;\n"
        "   float quadratic;\n"
        "\n"
        "   vec3 ambient;\n"
        "   vec3 diffuse;\n"
        "   vec3 specular;\n"
        "};\n";
    const std::string spotlight_light_struct =
        "struct SpotLight {\n"
        "   vec3 position;\n"
        "   vec3 direction;\n"
        "   float cutOff;\n"
        "   float outerCutOff;\n"
        "   \n"
        "   float constant;\n"
        "   float linear;\n"
        "   float quadratic;\n"
        "\n"
        "   vec3 ambient;\n"
        "   vec3 diffuse;\n"
        "   vec3 specular;\n"
        "};\n";

    const std::string num_dir_lights_def = "#define NR_DIR_LIGHTS " + std::to_string(numDirLights) + "\n";
    const std::string num_point_lights_def = "#define NR_POINT_LIGHTS " + std::to_string(numPointLights) + "\n";
    const std::string num_spot_lights_def = "#define NR_SPOT_LIGHTS " + std::to_string(numSpotLights) + "\n";

    // START CONSTRUCTING SHADER
    const std::string shader_begin =
        opengl_version +
        material_struct +
        "out vec4 "+this->fragmentOutColour+";\n"
        "\n"
        "in vec3 FragPos;\n"
        "in vec3 Normal;\n"
        "\n"
        ""+texture_import+"\n";

    std::string lighting_defs = "";
    std::string lighting_count_defs = "";
    std::string uniform_defs =
        "uniform vec3 viewPos;\n"
        "uniform Material material;\n";
    std::string lighting_calc = "";
    std::string lighting_fwd_defs = "";
    std::string lighting_funcs = "";

    if (numDirLights > 0) {
        lighting_defs += direction_light_struct;
        lighting_count_defs += num_dir_lights_def;
        uniform_defs += "uniform DirLight dirLights[NR_DIR_LIGHTS];\n";
        lighting_calc +=
            "   for (int i = 0; i < NR_DIR_LIGHTS; i++) {\n"
            "      result += CalcDirLight(dirLights[i], norm, viewDir);\n"
            "   }\n";
        lighting_fwd_defs += calcDirLight_fwd;
        lighting_funcs += calcDirLight(texture_diffuse, texture_specular);
    }

    if (numPointLights > 0) {
        lighting_defs += point_light_struct;
        lighting_count_defs += num_point_lights_def;
        uniform_defs += "uniform PointLight pointLights[NR_POINT_LIGHTS];\n";
        lighting_calc +=
            "   for (int i = 0; i < NR_POINT_LIGHTS; i++) {\n"
            "      result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"
            "   }\n";
        lighting_fwd_defs += calcPointLight_fwd;
        lighting_funcs += calcPointLight(texture_diffuse, texture_specular);
    }

    if (numSpotLights > 0) {
        lighting_defs += spotlight_light_struct;
        lighting_count_defs += num_spot_lights_def;
        uniform_defs += "uniform SpotLight spotLights[NR_SPOT_LIGHTS];\n";
        lighting_calc +=
            "   for (int i = 0; i < NR_SPOT_LIGHTS; i++) {\n"
            "      result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);\n"
            "   }\n";
        lighting_fwd_defs += calcSpotLight_fwd;
        lighting_funcs += calcSpotLight(texture_diffuse, texture_specular);
    }

    const std::string shader_main =
        "void main() {\n"
        "   vec3 norm = normalize(Normal);\n"
        "   vec3 viewDir = normalize(viewPos - FragPos);\n"
        "   vec3 result = vec3(0.0, 0.0, 0.0);\n"
        ""+lighting_calc+"\n"
        "   "+this->fragmentOutColour+" = vec4(result, 1.0);\n"
        "}\n";
    return
        shader_begin+
        lighting_defs+
        lighting_count_defs+
        uniform_defs+
        texture_shaders+
        // Fwd Declare lights
        lighting_fwd_defs+
        // Call main
        shader_main +
        // Define functions.
        lighting_funcs;
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

bool Mesh::autoCreateShader(Engine* engine) {
	// Recount texture types.
	this->diffuseNr = countNumTextureType(this->textures, this->diffuseDesc);
	this->specularNr = countNumTextureType(this->textures, this->specularDesc);
	this->normalNr = countNumTextureType(this->textures, this->normalDesc);
	this->heightNr = countNumTextureType(this->textures, this->heightDesc);

    // dir, point, spotlight, flashlight
    const auto dirCount = engine->getLightManager()->getDirLights().size();
    const auto pointCount = engine->getLightManager()->getPointLights().size();
    const auto spotlightCount = engine->getLightManager()->getSpotLight().size();
    const auto flashlightCount = engine->getLightManager()->getFlashLight().size();

	const std::string vertex_code = this->create_vertex_shader();
    // Flashlights are implemented as spotlights.
    const std::string fragment_code = this->create_fragment_shader(dirCount, pointCount, spotlightCount + flashlightCount);

    this->shader = Shader(vertex_code, fragment_code);
    const bool is_valid = this->shader.valid();

#if ENGINE_DEBUG
	if (is_valid) {
		std::cout << "Shader compiled successfully" << std::endl;
	} else {
		std::cout << "Computed Shader::VERTEX --" << "\n" << vertex_code << "\n -- END VERTEX" << std::endl;
		std::cout << "Computed Shader::FRAGMENT --" << "\n" << fragment_code << "\n -- END FRAGMENT" << std::endl;
	}
#endif
    if (is_valid) {
        // Set light properties.
        this->shader.use();

        for (std::size_t i = 0; i < dirCount; ++i) {
            const auto dirLight = engine->getLightManager()->getDirLights().at(i);
            this->shader.setVec3("dirLights["+std::to_string(i)+"].direction", dirLight.direction);

            this->shader.setVec3("dirLights["+std::to_string(i)+"].ambient", dirLight.ambient);
            this->shader.setVec3("dirLights["+std::to_string(i)+"].diffuse", dirLight.diffuse);
            this->shader.setVec3("dirLights["+std::to_string(i)+"].specular", dirLight.specular);
        }

        for (std::size_t i = 0; i < pointCount; ++i) {
            const auto pointLight = engine->getLightManager()->getPointLights().at(i);
            this->shader.setVec3("pointLights["+std::to_string(i)+"].position", pointLight.position);

            this->shader.setFloat("pointLights["+std::to_string(i)+"].constant", pointLight.constant);
            this->shader.setFloat("pointLights["+std::to_string(i)+"].linear", pointLight.linear);
            this->shader.setFloat("pointLights["+std::to_string(i)+"].quadratic", pointLight.quadratic);

            this->shader.setVec3("pointLights["+std::to_string(i)+"].ambient", pointLight.ambient);
            this->shader.setVec3("pointLights["+std::to_string(i)+"].diffuse", pointLight.diffuse);
            this->shader.setVec3("pointLights["+std::to_string(i)+"].specular", pointLight.specular);
        }

        for (std::size_t i = 0; i < spotlightCount + flashlightCount; ++i) {
            // First half is spotlights, then flashlights.
            if (i < spotlightCount) {
                const auto index = i;
                const auto spotLight = engine->getLightManager()->getSpotLight().at(index);

                this->shader.setVec3("spotLights["+std::to_string(i)+"].position", spotLight.position);
                this->shader.setVec3("spotLights["+std::to_string(i)+"].direction", spotLight.direction);
                this->shader.setFloat("spotLights["+std::to_string(i)+"].cutOff", spotLight.cutOff);
                this->shader.setFloat("spotLights["+std::to_string(i)+"].outerCutOff", spotLight.outerCutOff);

                this->shader.setFloat("spotLights["+std::to_string(i)+"].constant", spotLight.constant);
                this->shader.setFloat("spotLights["+std::to_string(i)+"].linear", spotLight.linear);
                this->shader.setFloat("spotLights["+std::to_string(i)+"].quadratic", spotLight.quadratic);

                this->shader.setVec3("spotLights["+std::to_string(i)+"].ambient", spotLight.ambient);
                this->shader.setVec3("spotLights["+std::to_string(i)+"].diffuse", spotLight.diffuse);
                this->shader.setVec3("spotLights["+std::to_string(i)+"].specular", spotLight.specular);
            } else {
                // Update flashlights
                const auto index = i - spotlightCount;
                const auto flashLight = engine->getLightManager()->getFlashLight().at(index);

                this->shader.setVec3("flashLights["+std::to_string(i)+"].position", flashLight.position);
                this->shader.setVec3("flashLights["+std::to_string(i)+"].direction", flashLight.direction);
                this->shader.setFloat("flashLights["+std::to_string(i)+"].cutOff", flashLight.cutOff);
                this->shader.setFloat("flashLights["+std::to_string(i)+"].outerCutOff", flashLight.outerCutOff);

                this->shader.setFloat("flashLights["+std::to_string(i)+"].constant", flashLight.constant);
                this->shader.setFloat("flashLights["+std::to_string(i)+"].linear", flashLight.linear);
                this->shader.setFloat("flashLights["+std::to_string(i)+"].quadratic", flashLight.quadratic);

                this->shader.setVec3("flashLights["+std::to_string(i)+"].ambient", flashLight.ambient);
                this->shader.setVec3("flashLights["+std::to_string(i)+"].diffuse", flashLight.diffuse);
                this->shader.setVec3("flashLights["+std::to_string(i)+"].specular", flashLight.specular);
            }
        }
    }

    return is_valid;
}

std::string Mesh::description() const {
    return "Num Diffuse: (" + this->diffuseDesc + ") - " + std::to_string(this->diffuseNr) + "\n" +
        "Num Specular: (" + this->specularDesc + ") - " + std::to_string(this->specularNr) + "\n" +
        "Num Normal: (" + this->normalDesc + ") - " + std::to_string(this->normalNr) + "\n" +
        "Num Height: (" + this->heightDesc + ") - " + std::to_string(this->heightNr) + "\n";
}

void Mesh::UpdatePerspective(Engine* engine) {
	this->shader\
		.use()\
		.setMat4("projection", engine->get3DRenderer()->getProjection())\
        .setMat4("view", engine->get3DRenderer()->getView())\
        .setVec3("viewPos", engine->get3DRenderer()->getCameraPos());

    // Update light position
    const auto dirCount = engine->getLightManager()->getDirLights().size();
    const auto pointCount = engine->getLightManager()->getPointLights().size();
    const auto spotlightCount = engine->getLightManager()->getSpotLight().size();
    const auto flashlightCount = engine->getLightManager()->getFlashLight().size();

    for (std::size_t i = 0; i < dirCount; ++i) {
        const auto dirLight = engine->getLightManager()->getDirLights().at(i);
        this->shader.setVec3("dirLights["+std::to_string(i)+"].direction", dirLight.direction);
    }

    for (std::size_t i = 0; i < pointCount; ++i) {
        const auto pointLight = engine->getLightManager()->getPointLights().at(i);
        this->shader.setVec3("pointLights["+std::to_string(i)+"].position", pointLight.position);
    }

    for (std::size_t i = 0; i < spotlightCount + flashlightCount; ++i) {
        // First half is spotlights, then flashlights.
        if (i < spotlightCount) {
            const auto index = i;
            const auto spotLight = engine->getLightManager()->getSpotLight().at(index);

            this->shader.setVec3("spotLights["+std::to_string(i)+"].position", spotLight.position);
            this->shader.setVec3("spotLights["+std::to_string(i)+"].direction", spotLight.direction);
        } else {
            const auto index = i - spotlightCount;
            const auto flashLight = engine->getLightManager()->getFlashLight().at(index);

            this->shader.setVec3("flashLights["+std::to_string(i)+"].position", flashLight.position);
            this->shader.setVec3("flashLights["+std::to_string(i)+"].direction", flashLight.direction);
        }
    }
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
