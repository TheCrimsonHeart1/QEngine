#include "UI.h"
#include "TextureLoader.h"
#include "Sprite.h"
#include "imgui.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "AssetManager.h" // Provides assetFolder and AssetPath

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

extern lua_State* L;
bool RunLuaFile(const std::string& filepath); // forward declaration

void RenderGUI(std::vector<Sprite>& sprites) {
    // ==============================
    // Sprite Manager Window
    // ==============================
    ImGui::Begin("Sprite Manager");

    if (ImGui::BeginTabBar("MainTabBar")) {

        // --- Texture Loader Tab ---
        if (ImGui::BeginTabItem("Texture Loader")) {
            static char pathBuffer[256] = "textures/Meme.bmp"; // relative to assetFolder
            ImGui::InputText("Texture Path", pathBuffer, sizeof(pathBuffer));

            if (ImGui::Button("Load Texture")) {
                std::string fullPath = AssetPath(pathBuffer); // resolve full path
                GLuint tex = LoadTexture(fullPath.c_str());
                if (tex) {
                    sprites.push_back({tex, 100.0f, 100.0f, 128.0f, 128.0f});
                    std::cout << "Loaded texture: " << fullPath << std::endl;
                } else {
                    std::cerr << "Failed to load texture: " << fullPath << std::endl;
                }
            }

            ImGui::EndTabItem();
        }

        // --- Sprite Editor Tab ---
        if (ImGui::BeginTabItem("Sprite Editor")) {
            ImGui::Text("Sprite Editor:");
            ImGui::BeginChild("SpriteList", ImVec2(0, 300), true); // scrollable child

            for (size_t i = 0; i < sprites.size(); i++) {
                ImGui::PushID((int)i);

                ImGui::Text("Sprite %d", (int)i);
                ImGui::SliderFloat("X", &sprites[i].x, 0.0f, 1920.0f);
                ImGui::SliderFloat("Y", &sprites[i].y, 0.0f, 1080.0f);
                ImGui::SliderFloat("Width", &sprites[i].width, 10.0f, 400.0f);
                ImGui::SliderFloat("Height", &sprites[i].height, 10.0f, 400.0f);

                if (ImGui::Button("Delete")) {
                    glDeleteTextures(1, &sprites[i].textureID);
                    sprites.erase(sprites.begin() + i);
                    ImGui::PopID();
                    break; // stop iterating after deletion
                }

                ImGui::Separator();
                ImGui::PopID();
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // --- Lua Script Loader Tab ---
        if (ImGui::BeginTabItem("Lua Loader")) {
            static char luaPath[256] = "scripts/my_script.lua"; // relative path
            ImGui::InputText("Lua File Path", luaPath, sizeof(luaPath));

            if (ImGui::Button("Run")) {
                std::string fullLuaPath = AssetPath(luaPath); // resolve full path
                std::ifstream file(fullLuaPath);
                if (!file.good()) {
                    std::cerr << "Lua file not found: " << fullLuaPath << std::endl;
                } else {
                    if (RunLuaFile(fullLuaPath)) {
                        std::cout << "Lua file loaded: " << fullLuaPath << std::endl;
                    }
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End(); // End Sprite Manager

    // ==============================
    // Build & Export Window
    // ==============================
    ImGui::Begin("Build & Export");

    if (ImGui::Button("Run")) {
        const char* batchPath = "build_game.bat"; // <-- change path
        // Option 1: blocking
        int result = system(batchPath);
        if (result == 0)
            std::cout << "Build script ran successfully!" << std::endl;
        else
            std::cerr << "Build script failed. Error code: " << result << std::endl;

        // Option 2 (non-blocking, background):
        // std::string cmd = "start \"\" \"" + std::string(batchPath) + "\"";
        // system(cmd.c_str());
    }

    ImGui::End(); // End Build & Export
}
