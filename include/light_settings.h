#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

struct LightSettings {
    bool      isEnabled = true;
    float     rotationY = 0.0f;
    float     rotationX = 0.0f;
    float     distance  = 3.0f;
    glm::vec3 diffuse  = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float     intensity = 1.0f;
    bool      castsShadow = true;  // NEW
    float     shadowBias = 0.05f;  // NEW

        // Shadow resources
    GLuint    shadowMapFBO = 0;
    GLuint    shadowCubemap = 0;
};