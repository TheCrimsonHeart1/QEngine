#include "CodeEditor.h"
#include "imgui.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "AssetManager.h"  // Provides assetFolder and AssetPath

extern std::filesystem::path assetFolder;
extern std::string AssetPath(const std::string& relativePath);

void RenderCodeEditor(CodeEditor& editor, const char* title) {
    if (!editor.isOpen) return;

    static bool showFileBrowser = false; // persists between frames

    ImGui::Begin(title, &editor.isOpen, ImGuiWindowFlags_MenuBar);

    // --- Menu Bar ---
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Clear")) {
                editor.editor.SetText("");
            }

            if (ImGui::MenuItem("Load Script")) {
                showFileBrowser = true; // request popup to open
            }

            if (ImGui::MenuItem("Save Script")) {
                std::string path = AssetPath("scripts/main.lua"); // default save path
                std::ofstream file(path);
                if (file.is_open()) {
                    file << editor.editor.GetText();
                    std::cout << "Saved script to: " << path << std::endl;
                } else {
                    std::cerr << "Failed to save script: " << path << std::endl;
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // --- Open popup if requested ---
    if (showFileBrowser) {
        ImGui::OpenPopup("Lua File Browser");
        showFileBrowser = false;
    }

    // --- Render the syntax-highlighted editor ---
    editor.editor.Render(title);

    // --- File Browser Popup ---
    if (ImGui::BeginPopupModal("Lua File Browser", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::filesystem::path scriptsFolder = assetFolder / "scripts";

        if (std::filesystem::exists(scriptsFolder) && std::filesystem::is_directory(scriptsFolder)) {
            for (auto& entry : std::filesystem::directory_iterator(scriptsFolder)) {
                if (!entry.is_regular_file()) continue;

                auto path = entry.path();
                if (path.extension() == ".lua") {
                    if (ImGui::Selectable(path.filename().string().c_str())) {
                        std::ifstream file(path);
                        if (file.is_open()) {
                            std::string content((std::istreambuf_iterator<char>(file)),
                                                std::istreambuf_iterator<char>());
                            editor.editor.SetText(content);
                            std::cout << "Loaded script: " << path.string() << std::endl;
                        } else {
                            std::cerr << "Failed to open script: " << path.string() << std::endl;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
        } else {
            ImGui::Text("Scripts folder not found: %s", scriptsFolder.string().c_str());
        }

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}
