#pragma once
#include <string>
#include <vector>
#include <light_settings.h>

std::vector<std::string> loadModelFiles(const std::string& path);
void saveLightSetup(const std::vector<LightSettings>& lights, const std::string& filename);
std::vector<LightSettings> loadLightSetup(const std::string& filename);

