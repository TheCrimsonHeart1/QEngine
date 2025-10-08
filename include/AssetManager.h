#pragma once
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Declare the global variable (no definition)
extern fs::path assetFolder;

// Declare the function as inline to avoid multiple definitions
inline std::string AssetPath(const std::string& relativePath) {
    return (assetFolder / relativePath).string();
}
