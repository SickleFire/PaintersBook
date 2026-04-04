#pragma once
#include <glm/glm.hpp>

struct LightSettings {
    bool      isEnabled = true;
    float     rotationY = 0.0f;
    float     rotationX = 0.0f;
    float     distance  = 3.0f;
    glm::vec3 diffuse  = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float     intensity = 1.0f;
};