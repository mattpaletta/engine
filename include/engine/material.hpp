#pragma once

struct Material {
    glm::vec3 DiffuseColour;
    glm::vec3 SpecularColour;
    glm::vec3 AmbientColour;
    glm::vec3 EmissiveColour;
    glm::vec3 TransparentColour;
    float shininess; // for blong-phinn
    float ambient_tex_blend; // amount to mix colour with texture (if applicable)
    float diffuse_tex_blend; // amount to mix colour with texture (if applicable)
    float specular_tex_blend; // amount to mix colour with texture (if applicable)
};
