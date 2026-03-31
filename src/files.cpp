#include <filesystem>
#include <vector>
#include <string>

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

