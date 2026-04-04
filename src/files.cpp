#include <filesystem>
#include <vector>
#include <string>
#include <json.hpp>
#include <fstream>
#include "files.h"
#include <glm/glm.hpp>
using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<std::string> loadModelFiles(const std::string& path) {
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            auto ext = entry.path().extension().string();
            // filter by extension if needed
            if (ext == ".obj" || ext == ".glb" || ext == ".fbx") {
                files.push_back(entry.path().string());
            }
        }
    }
    return files;
}

void saveLightSetup(const std::vector<LightSettings>& lights, const std::string& filename) {
    json j;
    j["lights"] = json::array();

    for (const auto& light : lights) {
        j["lights"].push_back({
            {"isEnabled", light.isEnabled},
            {"rotationX", light.rotationX},
            {"rotationY", light.rotationY},
            {"distance",  light.distance},
            {"intensity", light.intensity},
            {"diffuse",   {light.diffuse.x,  light.diffuse.y,  light.diffuse.z}},
            {"specular",  {light.specular.x, light.specular.y, light.specular.z}},
        });
    }

    std::ofstream file(filename);
    file << j.dump(4); // pretty print
}

std::vector<LightSettings> loadLightSetup(const std::string& filename) {
    std::vector<LightSettings> lights;

    std::ifstream file(filename);
    if (!file.is_open()) return lights; // file not found, return empty

    json j;
    file >> j;

    for (const auto& jlight : j["lights"]) {
        LightSettings light;
        light.isEnabled = jlight["isEnabled"];
        light.rotationX = jlight["rotationX"];
        light.rotationY = jlight["rotationY"];
        light.distance  = jlight["distance"];
        light.intensity = jlight["intensity"];
        light.diffuse   = glm::vec3(jlight["diffuse"][0], jlight["diffuse"][1], jlight["diffuse"][2]);
        light.specular  = glm::vec3(jlight["specular"][0], jlight["specular"][1], jlight["specular"][2]);
        lights.push_back(light);
    }

    return lights;
}
