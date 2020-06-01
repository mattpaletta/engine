#pragma once

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight {
    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

using FlashLight = SpotLight;

class LightManager {
public:
    LightManager() = default;
    ~LightManager() = default;

    void AddPointLight(const PointLight& pointLight) {
        this->point.push_back(pointLight);
    }

    std::vector<PointLight>& getPointLights() {
        return this->point;
    }

    std::vector<DirLight>& getDirLights() {
        return this->direction;
    }

    void AddDirectionLight(const DirLight& dirLight) {
        this->direction.push_back(dirLight);
    }

    std::vector<SpotLight>& getSpotLight() {
        return this->spotlight;
    }

    void AddSpotLight(const SpotLight& spotLight) {
        this->spotlight.push_back(spotLight);
    }

    std::vector<FlashLight>& getFlashLight() {
        return this->flashlight;
    }

    void AddFlashLight(const FlashLight& flashLight) {
        this->flashlight.push_back(flashLight);
    }

    std::size_t getLightCount() const {
        return this->point.size() + this->direction.size() + this->spotlight.size() + this->flashlight.size();
    }

private:
    std::vector<PointLight> point;
    std::vector<DirLight> direction;
    std::vector<SpotLight> spotlight;
    std::vector<FlashLight> flashlight;
};
